#include "WebServer.h"

WebServer::WebServer(
            int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            m_port(port), m_openLinger(OptLinger), m_timeoutMS(timeoutMS), m_isClose(false),
            m_timer(new HeapTimer()), m_threadpool(new ThreadPool(threadNum)), m_epoller(new Epoller())
{
    m_srcDir = getcwd(nullptr, 256);
    assert(m_srcDir);

    strncat(m_srcDir, "/resources/", 16);
    HttpConn::m_userCount = 0;
    HttpConn::m_srcDir = m_srcDir;
    // 初始化mysql数据库连接池
    SqlConnPool::GetInstance()->init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);
    // 初始化 TODO:
    initEventMode(trigMode);
    // 初始化socket
    if (!initSocket())
    {
        m_isClose = true;
        LOG_ERROR("WebServer init socket failed!");
    }

    // 开启日志
    if (openLog)
    {
        Log::GetInstance()->init(logLevel, "./log", ".log", logQueSize);
        if (m_isClose)
        {
            LOG_ERROR("============ Server init error! ============");
        }
        else
        {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", m_port, OptLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (m_listenEvent & EPOLLET ? "ET": "LT"),
                            (m_connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::m_srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    close(m_listenFd);
    m_isClose = true;
    free(m_srcDir);
    SqlConnPool::GetInstance()->close();
}

void WebServer::initEventMode(int trigMode) {
    m_listenEvent = EPOLLRDHUP;
    m_connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        m_connEvent |= EPOLLET;
        break;
    case 2:
        m_listenEvent |= EPOLLET;
        break;
    case 3:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    default:
        m_listenEvent |= EPOLLET;
        m_connEvent |= EPOLLET;
        break;
    }
    HttpConn::m_isET = (m_connEvent & EPOLLET);
}

void WebServer::start()
{
    int timeMS = -1;

    if (m_isClose)
    {
        return ;
    }

    LOG_INFO("========== Server start ==========");

    while (!m_isClose)
    {
        if (m_timeoutMS > 0)
        {
            timeMS = m_timer->GetNextTick();
        }

        // 等待socket事件
        int eventCounts = m_epoller->wait(timeMS);
        for (int i = 0; i < eventCounts; i++)
        {
            // 处理事件
            int fd = m_epoller->getEventFd(i);
            uint32_t events = m_epoller->getEvents(i);

            // 监听socket连接
            if (fd == m_listenFd)
            {
                dealListen();
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                assert(m_clientHttpConn.count(fd) > 0);
                closeConn(&m_clientHttpConn[fd]);
            }
            // socket接收到消息
            else if (events & EPOLLIN)
            {
                assert(m_clientHttpConn.count(fd) > 0);
                dealRead(&m_clientHttpConn[fd]);
            }
            else if (events & EPOLLOUT)
            {
                assert(m_clientHttpConn.count(fd) > 0);
                dealWrite(&m_clientHttpConn[fd]);
            }
            else
            {
                LOG_ERROR("Unexpected event!");
            }
        }
    }
}

void WebServer::sendError(int fd, const char* info)
{
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0)
    {
        LOG_WARN("send error info to client[%d] failed!", fd);
    }
    close(fd);
}

void WebServer::closeConn(HttpConn* client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    m_epoller->delFd(client->getFd());
    client->closeConn();
}

void WebServer::addClient(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    // 为客户端建立Http连接
    m_clientHttpConn[fd].init(fd, addr);
    if (m_timeoutMS > 0)
    {
        // 添加超时自动关闭连接定时器
        m_timer->add(fd, m_timeoutMS, std::bind(&WebServer::closeConn, this, &m_clientHttpConn[fd]));
        // printf("add timer success, fd = %d\n", fd);
    }

    // 监听客户端的消息
    m_epoller->addFd(fd, EPOLLIN | m_connEvent);
    setFdNonblock(fd);

    LOG_INFO("Client[%d] in!", m_clientHttpConn[fd].getFd());
}

void WebServer::dealListen()
{
    struct sockaddr_in addr = {0};
    socklen_t sock_len = sizeof(sockaddr_in);

    do
    {
        int fd = accept(m_listenFd, (struct sockaddr *)&addr, &sock_len);
        if (fd <= 0)
        {
            return ;
        }
        else
        {
            // 超出最大客户数量
            if (HttpConn::m_userCount >= MAX_FD)
            {
                sendError(fd, "Server Busy!");
                LOG_WARN("Clients are full");
                return ;
            }
        }

        // if (m_clientHttpConn.count(fd) > 0)
        // {
        //     continue;
        // }
        addClient(fd, addr);
    } while (m_listenEvent & EPOLLET);

}

void WebServer::dealRead(HttpConn* client)
{
    assert(client);
    extentTime(client);
    // 使用线程池处理客户端的请求
    m_threadpool->addTask(std::bind(&WebServer::onRead, this, client));
}

void WebServer::dealWrite(HttpConn* client)
{
    assert(client);
    extentTime(client);
    // 使用线程池向客户端发送Http应答
    m_threadpool->addTask(std::bind(&WebServer::onWrite, this, client));
}

void WebServer::extentTime(HttpConn* client)
{
    assert(client);
    if (m_timeoutMS > 0)
    {
        m_timer->adjust(client->getFd(), m_timeoutMS);
    }
}

void WebServer::onRead(HttpConn* client)
{
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN)
    {
        closeConn(client);
        return ;
    }

    onProcess(client);
}

void WebServer::onProcess(HttpConn* client)
{
    assert(client);
    if (client->process())
    {
        m_epoller->modFd(client->getFd(), m_connEvent | EPOLLOUT);
        // m_epoller->modFd(client->getFd(), EPOLLOUT);
    }
    else
    {
        m_epoller->modFd(client->getFd(), m_connEvent | EPOLLOUT);
        // m_epoller->modFd(client->getFd(), EPOLLIN);
    }
}

void WebServer::onWrite(HttpConn* client)
{
    assert(client);
    int ret = -1;
    int wirteErrno = 0;
    ret = client->write(&wirteErrno);
    if (client->ToWriteBytes() == 0)
    {
        // 传输完成
        if (client->isKeepAlive())
        {
            onProcess(client);
            return ;
        }
    }
    // TODO: 
    else if (ret < 0)
    {
        if (wirteErrno == EAGAIN)
        {
            // 继续传输
            m_epoller->modFd(client->getFd(), m_connEvent | EPOLLOUT);
            return ;
        }
    }

    closeConn(client);
}

bool WebServer::initSocket()
{
    int ret;
    struct sockaddr_in serverAddr;
    if (m_port > 65535 || m_port < 1024)
    {
        LOG_ERROR("Server Port: %d error!", m_port);
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(m_port);

    struct linger opeLinger = {0};
    // 如果开启优雅关闭
    if (m_openLinger)
    {
        opeLinger.l_onoff = 1;
        opeLinger.l_linger = 1;
    }

    // 创建socket
    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0)
    {
        LOG_ERROR("Create listen socket failed!");
        return false;
    }

    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &opeLinger, sizeof(opeLinger));
    if (ret < 0)
    {
        close(m_listenFd);
        LOG_ERROR("Init linger error!");
        return false;
    }

    int optval = 1;
    // 端口复用
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if (ret == -1)
    {
        LOG_ERROR("set socket setsockopt error!");
        close(m_listenFd);
        return false;
    }

    // 绑定
    ret = bind(m_listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        close(m_listenFd);
        LOG_ERROR("Bind Port:%d failed!", m_port);
        return false;
    }

    ret = listen(m_listenFd, 6);
    if (ret < 0)
    {
        close(m_listenFd);
        LOG_ERROR("Listen port:%d failed!", m_port);
        return false;
    }

    // 将listenfd 加入Epoll
    ret = m_epoller->addFd(m_listenFd, m_listenEvent | EPOLLIN);
    if (ret == 0)
    {
        LOG_ERROR("add listenFd to epoll failed!");
        close(m_listenFd);
        return false;
    }

    setFdNonblock(m_listenFd);
    LOG_INFO("Server port:%d", m_port);
    return true;
}

int WebServer::setFdNonblock(int fd)
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

