#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
 * 实现RAII的写优先的读写锁
 */
class RWlock{
public:
    explicit RWlock(): reader(0), writeQuest(0), writer(false){}

    std::mutex mtx;
    std::condition_variable readCV;
    std::condition_variable writeCV;
    int reader;
    int writeQuest;
    bool writer;
};


class ReaderLockRAII{
public:
    explicit ReaderLockRAII(RWlock &lock): lock_(lock){
        std::unique_lock<std::mutex> locker(lock_.mtx);
        while(lock_.writeQuest > 0 || lock_.writer)
            lock_.readCV.wait(locker);
        lock_.reader ++;
    }
    ~ReaderLockRAII(){
        std::unique_lock<std::mutex> locker(lock_.mtx);
        lock_.reader --;
        if(lock_.reader == 0)
            lock_.writeCV.notify_one();
    }
private:
    RWlock &lock_;
};

class WriterLockRAII{
public:
    explicit WriterLockRAII(RWlock &lock): lock_(lock){
        std::unique_lock<std::mutex> locker(lock_.mtx);
        lock_.writeQuest ++;
        while(lock_.reader > 0 || lock_.writer)
            lock_.writeCV.wait(locker);
        lock_.writeQuest --;
        lock_.writer = true;
    }
    ~WriterLockRAII(){
        std::unique_lock<std::mutex> locker(lock_.mtx);
        lock_.writer = false;
        if(lock_.writeQuest > 0)
            lock_.writeCV.notify_one();
        else
            lock_.readCV.notify_all();
    }
private: 
    RWlock &lock_;
};