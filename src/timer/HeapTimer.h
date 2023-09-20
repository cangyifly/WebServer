#ifndef SRC_TIMER_HEAPTIMER_H
#define SRC_TIMER_HEAPTIMER_H

#include <chrono>
#include <vector>
#include <assert.h>
#include <unordered_map>
#include <algorithm>
#include <functional>

#include "Log.h"

#define HEAP_DEFAULT_CAPACITY 64

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
    int id;
    TimeStamp expires;
    TimeoutCallBack task;
    bool operator<(const TimerNode& t)
    {
        return expires < t.expires;
    }
};

class HeapTimer
{
private:
    // 模拟小根堆数组
    std::vector<TimerNode> m_heap;
    // 节点id到数组下标映射
    std::unordered_map<int, size_t> m_ref;

private:
    void del(size_t i);
    
    void shiftup(size_t cur);

    bool shiftdown(size_t cur, size_t n);

    void swapNode(size_t i, size_t j);

public:
    // 修改vector的capactity为64
    HeapTimer() { m_heap.reserve(64); }
    ~HeapTimer() { clear(); }

    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& task);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();
};

#endif