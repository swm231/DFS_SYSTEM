#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <list>

#include "../single/epoll.h"
#include "../http/httpconn.h"
#include "../pool/threadpool.h"

class StorageMess;
class HttpConn;
class TrackerServer{
public:
    TrackerServer();
    ~TrackerServer();

    void StartUp();
    void CloseServer(int signum);

private:

    void DealRead_(BaseNode*);
    void DealWrite_(BaseNode*);

    void OnRead_(BaseNode*);
    void OnWrite_(BaseNode*);
    void DealClose_(BaseNode*);

    bool stop_;

    ListenHttp *listenhttp_;
    ListenTask *listentask_;

    std::unordered_map<int, HttpConn> users_;
};