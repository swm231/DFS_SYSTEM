#pragma once

#include "node.h"
#include "storagepack.h"
#include "../lock/rwlock.h"

class Message{
public:
    static const char CR[];
    static const char CRLF[];
    static std::unordered_map<int, StorageNode*> storageNode;
    static std::unordered_map<std::string, std::list<int> > group;
    static uint64_t timeUnitl;
    static bool flag;
    static RWlock lock;

    static std::unique_ptr<std::thread> changeFlagThread;
    static std::atomic<bool> threadIsRun;

    static void synPULGroup(){
        ReaderLockRAII lcoker(lock);
        for(auto it: group["public"])
            storageNode[it]->Status = STORAGE_STATUS_SYNC_RCVD;
        changeFLAG();
    }

    // 调用此函数前，保证已加写锁
    static void changeFLAG(){
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::seconds seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
        Message::timeUnitl = seconds_since_epoch.count() + Conf::timeOut + 5;
        Message::flag = true;
    }

    static void threadGO(){
        if(threadIsRun.load())
            return;
        threadIsRun = true;
        changeFlagThread = std::make_unique<std::thread>(&Message::updateFlag_);
    }

private:
    static void updateFlag_(){
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::seconds seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
        uint64_t Now = seconds_since_epoch.count(), sleepTime;
        {
            ReaderLockRAII locker(lock);
            sleepTime = timeUnitl - Now + 1;
        }
        while(sleepTime > 0){
            std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            std::chrono::seconds seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
            {
                ReaderLockRAII locker(lock);
                sleepTime = timeUnitl - Now + 1;
            }
        }
        WriterLockRAII locker(lock);
        flag = false;
        threadIsRun = false;
    }
};