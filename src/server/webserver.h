#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unordered_map>

#include "../single/epoll.h"
#include "../single/heaptimer.h"
#include "../pool/threadpool.h"
#include "../http/httpconn.h"

extern Epoll& globalEpoll();
extern ThreadPool& globalThreadPool();
extern HeapTimer& globalHeapTimer();

class WebServer{
public:
    WebServer(int port, int timeoutMS, int cookieOut, const char *host,
        const char *username, const char *pwd, const char *dbname);
    ~WebServer();

    void startUp();

    static int _cookieOut;
private:
    bool InitListenSocket();

    void dealNew_();
    void dealRead_(HttpConn *client);
    void dealWrite_(HttpConn *client);

    void closeConn_(HttpConn *client);

    void OnRead_(HttpConn *client);
    void OnWrite_(HttpConn *client);

    void updateTimer_(HttpConn *client);

    uint32_t listenEvent_;
    uint32_t connEvent_;

    bool stop_;

    int listenFd_;
    int port_;

    int timeoutMS_;

    std::unordered_map<int, HttpConn> users_;
};