#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <signal.h>

#include "../single/epoll.h"
#include "../single/heaptimer.h"
#include "../pool/threadpool.h"
#include "../http/httpconn.h"

class WebServer{
public:
    WebServer(int port, int timeoutMS, const char *host, const char *username, const char *pwd,
            const char *dbname, bool OpenLog, int Loglevel, int cookieOut);
    ~WebServer();

    void startUp();

    static volatile sig_atomic_t stop_;
    static void CloseServer(int signum);

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

    int listenFd_;
    int port_;

    int timeoutMS_;

    std::unordered_map<int, HttpConn> users_;
};