#ifndef SRC_SERVER_WEBSERVER_H
#define SRC_SERVER_WEBSERVER_H

#include <unistd.h>

#include "HttpConn.h"
#include "HeapTimer.h"
#include "ThreadPool.h"
#include "Log.h"
#include "SqlConnPool.h"
#include "Epoller.h"

class WebServer 
{
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    void start();

private:
    bool initSocket(); 
    void initEventMode(int trigMode);
    void addClient(int fd, sockaddr_in addr);
  
    void dealListen();
    void dealWrite(HttpConn* client);
    void dealRead(HttpConn* client);

    void sendError(int fd, const char*info);
    void extentTime(HttpConn* client);
    void closeConn(HttpConn* client);

    void onRead(HttpConn* client);
    void onWrite(HttpConn* client);
    void onProcess(HttpConn* client);

    static const int MAX_FD = 65536;

    static int setFdNonblock(int fd);

    // 服务器端口
    int m_port;
    // TCP断开连接
    bool m_openLinger;
    // 定时器默认超时时间，毫秒MS
    int m_timeoutMS;
    bool m_isClose;
    // 服务器监听
    int m_listenFd;
    char* m_srcDir;
    
    uint32_t m_listenEvent;
    uint32_t m_connEvent;
    
    // 小根堆定时器，用户关闭超时TCP连接
    std::unique_ptr<HeapTimer> m_timer;
    // 线程池
    std::unique_ptr<ThreadPool> m_threadpool;
    std::unique_ptr<Epoller> m_epoller;
    std::unordered_map<int, HttpConn> m_clientHttpConn;
};


#endif //WEBSERVER_H