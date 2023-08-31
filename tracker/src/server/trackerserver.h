#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <list>

#include "../single/epoll.h"
#include "../http/httpconn.h"
#include "../pool/threadpool.h"
#include "../single/heaptimer.h"

class StorageMess;
class HttpConn;
class TrackerServer{
public:
    TrackerServer();
    ~TrackerServer();

    void StartUp();
    void CloseServer(int signum);

private:

    void DealRead_(FdNode*);
    void DealWrite_(FdNode*);

    void OnRead_(FdNode*);
    void OnWrite_(FdNode*);
    void DealClose_(FdNode*);

    bool stop_;

    ListenHttp *listenhttp_;
    ListenTask *listentask_;

    std::unordered_map<int, HttpConn> users_;
};