#pragma once

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <atomic>
#include <unistd.h>

#include "httprequest.h"
#include "httpresponse.h"
#include "../single/threadpool.h"

class HttpConn{
public:
    HttpConn();
    ~HttpConn();

    void Init(int fd, const sockaddr_in &addr);
    void Close();
    
    void Read();
    bool parse();
    void process();

    void Send();
    
    std::string GetFileType();

    int GetFd() const;

    static std::atomic<int> userCount;
    static std::string srcDir_;

private:
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;

    HttpRequest request_;
    HttpResponse response_;
};