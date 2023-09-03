#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <list>

#include "basenode.h"
#include "config.h"

class StorageNode: public BaseNode{
public:
    StorageNode(){}
    StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    ~StorageNode();

    void Close(){ }

    uint16_t TaskPort, HttpPort;
    uint32_t TotalSize, UsedSize;
    int Status;
    std::string groupName;
private:
    void SignalAll_(int signum);

    void ParseHead_();

    void DealNEW_(const char*);
    void DealKEP_(const char*);
    void DealCHG_(const char*);
};

class ListenHttp: public BaseNode{
public:
    ListenHttp(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenHttp(){  }

    void Close(){  }
    void CloseUser(BaseNode *ptr);
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
