#pragma once

#include <unordered_map>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <algorithm>

#include "basenode.h"
#include "../single/encipher.h"
#include "../log/log.h"
#include "../single/epoll.h"


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

    void ReadyStoNEW();
    void ReadySYN(BEHAVIOR, PATH, const std::string &username, const std::string &filename);

    static std::unordered_map<int, StorageNode*> group;

private:
    long long hasSentLen_, BodyLen;
    int fileMsgfd_;
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
