/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#include "HeapTimer.h"

void HeapTimer::shiftup(size_t i) {
    assert(i >= 0 && i < m_heap.size());
    size_t j = (i - 1) / 2;
    while(j >= 0) {
        if(m_heap[j] < m_heap[i]) { break; }
        swapNode(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void HeapTimer::swapNode(size_t i, size_t j) {
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());
    std::swap(m_heap[i], m_heap[j]);
    m_ref[m_heap[i].id] = i;
    m_ref[m_heap[j].id] = j;
} 

bool HeapTimer::shiftdown(size_t index, size_t n) {
    assert(index >= 0 && index < m_heap.size());
    assert(n >= 0 && n <= m_heap.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && m_heap[j + 1] < m_heap[j]) j++;
        if(m_heap[i] < m_heap[j]) break;
        swapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if(m_ref.count(id) == 0) {
        /* 新节点：堆尾插入，调整堆 */
        i = m_heap.size();
        m_ref[id] = i;
        m_heap.push_back({id, Clock::now() + MS(timeout), cb});
        shiftup(i);
    } 
    else {
        /* 已有结点：调整堆 */
        i = m_ref[id];
        m_heap[i].expires = Clock::now() + MS(timeout);
        m_heap[i].task = cb;
        if(!shiftdown(i, m_heap.size())) {
            shiftup(i);
        }
    }
}

void HeapTimer::doWork(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(m_heap.empty() || m_ref.count(id) == 0) {
        return;
    }
    size_t i = m_ref[id];
    TimerNode node = m_heap[i];
    node.task();
    del(i);
}

void HeapTimer::del(size_t index) {
    /* 删除指定位置的结点 */
    assert(!m_heap.empty() && index >= 0 && index < m_heap.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = m_heap.size() - 1;
    assert(i <= n);
    if(i < n) {
        swapNode(i, n);
        if(!shiftdown(i, n)) {
            shiftup(i);
        }
    }
    /* 队尾元素删除 */
    m_ref.erase(m_heap.back().id);
    m_heap.pop_back();
}

void HeapTimer::adjust(int id, int newExpires) {
    /* 调整指定id的结点 */
    assert(!m_heap.empty() && m_ref.count(id) > 0);
    m_heap[m_ref[id]].expires = Clock::now() + MS(newExpires);;
    shiftdown(m_ref[id], m_heap.size());
}

void HeapTimer::tick() {
    /* 清除超时结点 */
    if(m_heap.empty()) {
        return;
    }
    while(!m_heap.empty()) {
        TimerNode node = m_heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.task();
        pop();
    }
}

void HeapTimer::pop() {
    assert(!m_heap.empty());
    del(0);
}

void HeapTimer::clear() {
    m_ref.clear();
    m_heap.clear();
}

int HeapTimer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!m_heap.empty()) {
        res = std::chrono::duration_cast<MS>(m_heap.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}