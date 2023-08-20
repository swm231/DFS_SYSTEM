#pragma once

#include <sys/types.h>
#include <atomic>
#include <unistd.h>

#include "httprequest.h"
#include "httpresponse.h"

class HttpConn{
public:
    HttpConn();
    ~HttpConn();

    void Init(int fd, const sockaddr_in &addr);
    void Close();
    bool isClose();

    int read_process();
    int write_process();

    int GetFd() const;

    static std::atomic<int> userCount;
    static std::string srcDir_;

private:
    struct sockaddr_in addr_;

    bool isClose_;

    HttpMessage *Message_;
    HttpRequest request_;
    HttpResponse response_;
};