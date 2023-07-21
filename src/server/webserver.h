#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unordered_map>

#include "../single/epoll.h"
#include "../single/threadpool.h"
#include "../http/httpconn.h"

extern Epoll& globalEpoll();
extern ThreadPool& globalThreadPool();

class WebServer{
public:
    WebServer(int port);
    ~WebServer();

    void startUp();



private:
    void dealNew_();
    void dealRead_(HttpConn *client);
    void dealWrite_(HttpConn *client);

    void closeConn_(HttpConn *client);

    void OnRead_(HttpConn *client);
    void OnWrite_(HttpConn *client);

    uint32_t listenEvent_;
    uint32_t connEvent_;

    int listenFd_;

    std::unordered_map<int, HttpConn> users_;
};