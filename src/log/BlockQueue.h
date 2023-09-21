#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>
#include <assert.h>

#define MAX_CAPACITY 1000

template<class T>
class BlockQueue
{
public:
    explicit BlockQueue(size_t maxCapacity = MAX_CAPACITY);

    ~BlockQueue();

    void clear();

    bool empty();

    bool full();

    void close();

    size_t size();

    size_t capacity();

    T front();

    T back();

    void push_back(const T &item);

    void push_front(const T &item);

    bool pop(T &item);

    bool pop(T &item, int timeout);

    void flush();

private:
    std::deque<T> m_deq;

    size_t m_capacity;

    std::mutex m_mutex;

    bool m_isClosed;

    std::condition_variable m_condConsumer;

    std::condition_variable m_condProducer;
};

template<class T>
BlockQueue<T>::BlockQueue(size_t maxCapacity) : m_capacity(maxCapacity)
{
    assert(maxCapacity > 0);
    m_isClosed = false;
}

template<class T>
BlockQueue<T>::~BlockQueue()
{
    close();
}

template<class T>
void BlockQueue<T>::close()
{
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_deq.clear();
        m_isClosed = true;
    }

    m_condProducer.notify_all();
    m_condConsumer.notify_all();
}

template<class T>
void BlockQueue<T>::flush()
{
    m_condConsumer.notify_one();
}

template<class T>
void BlockQueue<T>::clear()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_deq.clear();
}

template<class T>
T BlockQueue<T>::front()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_deq.front();
}

template<class T>
T BlockQueue<T>::back()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_deq.back();
}

template<class T>
size_t BlockQueue<T>::size()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_deq.size();
}

template<class T>
size_t BlockQueue<T>::capacity()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_capacity;
}

template<class T>
void BlockQueue<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    while (m_deq.size() >= m_capacity)
    {
        m_condProducer.wait(locker);
    }

    m_deq.push_back(item);
    m_condConsumer.notify_one();
}

template<class T>
void BlockQueue<T>::push_front(const T& item)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    while (m_deq.size() >= m_capacity)
    {
        m_condProducer.wait(locker);
    }

    m_deq.push_front(item);
    m_condConsumer.notify_one();
}

template<class T>
bool BlockQueue<T>::empty()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_deq.empty();
}

template<class T>
bool BlockQueue<T>::full()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_deq.size() >= m_capacity;
}

template<class T>
bool BlockQueue<T>::pop(T& item)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    while (m_deq.empty())
    {
        m_condConsumer.wait(locker);
        if (m_isClosed)
        {
            return false;
        }
    }
    item = m_deq.front();
    m_deq.pop_front();
    m_condProducer.notify_one();
    return true;
}

template<class T>
bool BlockQueue<T>::pop(T& item, int timeout)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    while (m_deq.empty())
    {
        if (m_condConsumer.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
        {
            return false;
        }
        
        if (m_isClosed)
        {
            return false;
        }
    }
    item = m_deq.front();
    m_deq.pop_front();
    m_condProducer.notify_one();
    return true;
}
#endif