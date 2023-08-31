#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../http/httpconn.h"


class StorageServer{
public:
    StorageServer();
    ~StorageServer();

    void StartUp();
    void CloseServer(int signum);

private:

    bool stop_;

    void Connect_();

    void DealRead_(FdNode*);
    void DealWrite_(FdNode*);
    void OnRead_(FdNode*);
    void OnWrite_(FdNode*);
    void DealClose_(FdNode*);

    void KeepThrob_();

    std::unique_ptr<ListenHttp> listenhttp_;
    std::unique_ptr<ListenTask> listentask_;

    std::unique_ptr<std::thread> ThrobThread;
};