#include "heaptimer.h"

void HeapTimer::del_(size_t idx){
    if(heap_.empty() || idx >= heap_.size() || idx < 0)
        return;
    size_t i = idx, n = heap_.size() - 1;

    if(i < n){
        SwapNode_(i, n);
        if(!siftdown_(i, n))
            siftup_(i);
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::SwapNode_(size_t u, size_t v){
    std::swap(heap_[u], heap_[v]);
    ref_[heap_[u].id] = u;
    ref_[heap_[v].id] = v;
}

// 把 u 节点向上调整到符合堆性质的位置
void HeapTimer::siftup_(size_t idx){
    if(idx == 0) return;
    size_t f = (idx - 1) / 2;
    while(f >= 0){
        if(heap_[f] < heap_[idx]) break;
        SwapNode_(f, idx);
        idx = f;
        f = (idx - 1) / 2;
        if(idx == 0) break;
    }
}

// 在前 n 个节点中, 将 u 节点向下调整到合适位置,返回是否被调整
bool HeapTimer::siftdown_(size_t idx, size_t n){
    size_t i = idx;
    size_t j = i * 2 + 1;
    while(j < n){
        // 选择两个儿子中最小的那个
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j ++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > idx;
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack &cb){
    size_t i;
    // 新节点
    if(ref_.count(id) == 0){
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup_(i);
    }
    // 已有节点
    else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size()))
            siftup_(i);
    }
}

// 更新时间
void HeapTimer::update(int id, int timeout){
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);
    siftdown_(ref_[id], heap_.size());
}

// 关闭连接
void HeapTimer::close(int id){
    if(heap_.empty() || ref_.count(id) == 0)
        return;
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}

void HeapTimer::clear(){
    heap_.clear();
    ref_.clear();
}

// 清除超时节点
void HeapTimer::tick(){
    if(heap_.empty())
        return;
    while(!heap_.empty()){
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0)
            break;
        node.cb();
        pop();
    }
}

void HeapTimer::pop(){
    if(heap_.empty())
        return;
    del_(0);
}

int HeapTimer::GetNextTick(){
    tick();
    size_t res = -1;
    if(!heap_.empty()){
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) res = 0;
    }
    return res;
}