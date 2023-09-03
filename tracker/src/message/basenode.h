#pragma once

#include <iostream>
#include <netinet/in.h>

#include "../log/log.h"
#include "enumstatus.h"
#include "../buffer/buffer.h"
#include "../single/epoll.h"

class BaseNode{
public:
    BaseNode();
    BaseNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    BaseNode(BaseNode &&other);
    virtual ~BaseNode();

    virtual void Init();
    virtual void Close() = 0;

    virtual int ReadProcess();
    virtual int WriteProcess();

    uint32_t ip;
    uint16_t port, fd;
    int saveErrno;
    Buffer recvBuff, sendBuff;

    MSGSTATUS status;
    bool isOut;

    static bool InitListenSocket(uint16_t &sockt, int port);

private:
    virtual void ParseHead_() {  }
    virtual void ParseBody_() {  }
};