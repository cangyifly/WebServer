#include "HttpConn.h"

const char* HttpConn::m_srcDir;
std::atomic<int> HttpConn::m_userCount;
bool HttpConn::m_isET;

HttpConn::HttpConn()
{
    m_fd = -1;
    m_clientAddr = { 0 };
    m_isClose = true;
}

HttpConn::~HttpConn()
{
    closeConn();
}

void HttpConn::init(int sockFd, const sockaddr_in& clientAddr)
{
    assert(sockFd > 0);
    m_userCount++;
    m_clientAddr = clientAddr;
    m_fd = sockFd;
    m_writeBuff.clear();
    m_readBuff.clear();
    m_isClose = false;

    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", m_fd, getIP(), getPort(), (int)m_userCount);
}

void HttpConn::closeConn()
{
    m_response.unmapFile();
    if (!m_isClose)
    {
        m_isClose = true;
        m_userCount--;
        close(m_fd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", m_fd, getIP(), getPort(), (int)m_userCount);
    }
}

// 将m_fd文件中的内容读入m_readBuff
ssize_t HttpConn::read(int* saveErrno)
{
    ssize_t len = -1;
    do 
    {
        len = m_readBuff.readFd(m_fd, saveErrno);
        if (len <= 0)
        {
            break;
        }
    }
    while (m_isET);
    return len;
}

ssize_t HttpConn::write(int* saveErrno)
{
    ssize_t len = -1;
    do
    {
        len = writev(m_fd, m_iov, m_iovCnt);
        if (len <= 0)
        {
            *saveErrno = errno;
            break;
        }

        // 
        if (m_iov[0].iov_len + m_iov[1].iov_len == 0)
        {
            break;
        }
        // 写入了第一块 + 第二块
        else if (static_cast<size_t>(len) > m_iov[0].iov_len)
        {
            m_iov[1].iov_base = (uint8_t*)m_iov[1].iov_base + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len);

            if (m_iov[0].iov_len)
            {
                m_writeBuff.clear();
                m_iov[0].iov_len = 0;
            }
        }
        // 只写入了第一个数据块
        else
        {
            m_iov[0].iov_base = (uint8_t*)m_iov[0].iov_base + len;
            m_iov[0].iov_len -= len;
            m_writeBuff.retrive(len);
        }
    }
    while (m_isET || ToWriteBytes() > 10240);
    return len;
}

bool HttpConn::process()
{
    m_request.init();
    // 缓存无数据
    if (m_readBuff.readableBytes() <= 0)
    {
        return false;
    }
    
    if (m_request.parse(m_readBuff))
    {
        LOG_DEBUG("request : %s", m_request.path().c_str());
        m_response.init(m_srcDir, m_request.path(), m_request.isKeepAlive());
    }
    // 解析失败
    else
    {
        m_response.init(m_srcDir, m_request.path(),false, HttpResponse::BAD_REQUEST);
    }

    // 将Http Response写入缓存
    m_response.makeResponse(m_writeBuff);

    // 响应头
    m_iov[0].iov_base = const_cast<char*>(m_writeBuff.readPtr());
    m_iov[0].iov_len = m_writeBuff.readableBytes();
    m_iovCnt = 1;

    // 如果响应携带了文件
    if (m_response.getFileLen() > 0 && m_response.getFile())
    {
        m_iov[1].iov_base = m_response.getFile();
        m_iov[1].iov_len = m_response.getFileLen();
        m_iovCnt = 2;
    }

    LOG_DEBUG("filesize:%d, %d to %d", m_response.getFileLen(), m_iovCnt, ToWriteBytes());
    return true;
}




