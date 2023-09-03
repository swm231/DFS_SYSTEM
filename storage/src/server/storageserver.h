#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../http/httpconn.h"
#include "../pool/threadpool.h"

class StorageServer{
public:
    StorageServer();
    ~StorageServer();

    void StartUp();
    void CloseServer(int signum);

private:

    bool stop_;

    void Connect_();

    void DealRead_(BaseNode*);
    void DealWrite_(BaseNode*);
    void OnRead_(BaseNode*);
    void OnWrite_(BaseNode*);
    void DealClose_(BaseNode*);

    void KeepThrob_();

    std::unique_ptr<ListenHttp> listenhttp_;
    std::unique_ptr<ListenTask> listentask_;

    std::unique_ptr<std::thread> ThrobThread;
};