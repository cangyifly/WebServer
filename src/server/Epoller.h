#ifndef SRC_SERVER_EPOLLER_H
#define SRC_SERVER_EPOLLER_H

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>

#define EPOLLER_MAX_EVENT 1024

class Epoller 
{
public:
    explicit Epoller(int maxEvent = EPOLLER_MAX_EVENT);

    ~Epoller();

    bool addFd(int fd, uint32_t events);

    bool modFd(int fd, uint32_t events);

    bool delFd(int fd);

    int wait(int timeoutMs = -1);

    int getEventFd(size_t i) const;

    uint32_t getEvents(size_t i) const;
        
private:
    int m_epollFd;

    std::vector<struct epoll_event> m_events;    
};

#endif //EPOLLER_H