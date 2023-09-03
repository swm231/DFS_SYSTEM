#pragma once

#include <queue>
#include <unordered_map>
#include <functional>
#include <chrono>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

// 存储 套接字 死亡时间
struct TimerNode{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator <(const TimerNode &t){
        return expires < t.expires;
    }
};

// 小根堆 0号点是堆顶
// u的父亲: (u - 1) / 2
// u的左儿子: u * 2 + 1 右儿子: u * 2 + 2 
class HeapTimer{
public:
    static HeapTimer &Instance(){
        static HeapTimer instance;
        return instance;
    }

    void Init(){ isOpen_ = true; }

    void add(int id, int timeOut, const TimeoutCallBack &cb);

    void update(int id, int timeout);

    void close(int id);
    void erase(int id);

    void clear();

    void pop();

    void tick();

    int GetNextTick();

private:
    HeapTimer() : isOpen_(false){ heap_.reserve(64); }
    ~HeapTimer() { clear(); }

    void del_(size_t idx);
    void SwapNode_(size_t u, size_t v);

    void siftup_(size_t idx);
    bool siftdown_(size_t idx, size_t n);

    bool isOpen_;
    // 用数组模拟堆
    std::vector<TimerNode> heap_;
    // 根据 id 寻找节点在堆中的位置
    std::unordered_map<int, size_t> ref_;

};