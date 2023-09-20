#ifndef SRC_HTTP_HTTPCONN_H
#define SRC_HTTP_HTTPCONN_H

#include <atomic>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Buffer.h"
#include "Log.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    void init(int sockFd, const sockaddr_in& clientAddr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void closeConn();

    int getFd() const { return m_fd; };

    int getPort() const { return m_clientAddr.sin_port; };

    const char* getIP() const { return inet_ntoa(m_clientAddr.sin_addr); };
    
    sockaddr_in getClientAddr() const { return m_clientAddr; };
    
    bool process();

    int ToWriteBytes() 
    { 
        return m_iov[0].iov_len + m_iov[1].iov_len; 
    }

    bool isKeepAlive() const 
    {
        return m_request.isKeepAlive();
    }

    static bool m_isET;
    static const char* m_srcDir;
    static std::atomic<int> m_userCount;
    
private:
   
    int m_fd;
    struct sockaddr_in m_clientAddr;

    bool m_isClose;
    
    int m_iovCnt;
    struct iovec m_iov[2];
    
    Buffer m_readBuff; // 读缓冲区
    Buffer m_writeBuff; // 写缓冲区

    HttpRequest m_request;
    HttpResponse m_response;
};


#endif // SRC_HTTP_HTTPCONN_H