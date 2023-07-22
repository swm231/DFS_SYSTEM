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

    int read_process();
    int write_process();

    int Send(int *writeErrno);

    int GetFd() const;
    MSGSTATUS GetSendStatus(){
        return response_.HeadStatus;
    }
    bool IsKeepAlive(){
        return request_.IsKeepAlice();
    }

    static std::atomic<int> userCount;
    static std::string srcDir_;

private:
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;

    HttpRequest request_;
    HttpResponse response_;
};