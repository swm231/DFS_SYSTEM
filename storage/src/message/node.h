#pragma once

#include <unordered_map>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <algorithm>
#include <queue>

#include "basenode.h"
#include "../single/encipher.h"
#include "../log/log.h"
#include "../single/epoll.h"
#include "../consistlog/rollbacklog/rollbacklog.h"


class TaskNode: public BaseNode{
public:
    TaskNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    ~TaskNode();

    void Init();
    void Close();

private:
    void ParseHead_();
    void ParseBody_();

    void DealNEW_(const char*);
    void DealDEL_(const char*);
    void DealUPL_(const char*);

    std::string username_;
    long long hasSaveLen_, BodyLen_;
    FILE *fp;
    RollbackLog rollbacklog;
    uint64_t ringlogAddr_Rollback;
};

class TrackerNode : public BaseNode{
public:
    TrackerNode():BaseNode(){}
    TrackerNode(uint32_t _ip, uint16_t _port, int16_t _fd);
    TrackerNode(TrackerNode &&other);
    TrackerNode &operator=(const TrackerNode &other);
    ~TrackerNode();

    void Close() {  }

    void ReadyNEW();
    void ReadyKEP();
    void ReadySynSelf();
    void ReadySynUser(const std::string &username);

private:
    void ParseHead_();

    void DealTranew_();
    void DealTrakep_();

};

class StorageNode : public BaseNode{
public:
    StorageNode(): BaseNode(), fileMsgfd_(-1), hasSentLen_(0){}
    StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    StorageNode(StorageNode &&other);
    ~StorageNode();

    void Init();
    void Close();

    int WriteProcess();

    void AddTask(const storTaskPack &task);
    void AddTask(uint16_t port);
    void AddTask(const synPack &pack);

    static std::unordered_map<int, StorageNode*> group;

private:
    void ReadyNEW_(const storTaskPack &task);
    void ReadySyn_(const storTaskPack &task);

    void FinshSyn_(uint64_t ringLogId, uint64_t synLogId);
    std::function<void()> finshTask;

    long long hasSentLen_, BodyLen;
    int fileMsgfd_;

    std::mutex mtx_;
    std::queue<storTaskPack> taskQue_;
};


class ListenHttp: public BaseNode{
public:
    ListenHttp(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenHttp(){  }


    void Close(){  }
private:
    int ReadProcess();
    int WriteProcess();
};

class ListenTask: public BaseNode{
public:
    ListenTask(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenTask(){  }

    void Close(){  }
private:
    int ReadProcess();
    int WriteProcess();
};
