#include "httpconn.h"
extern Epoll& globalEpoll();

std::atomic<int> HttpConn::userCount;
std::string HttpConn::srcDir_;

HttpConn::HttpConn() : fd_(-1), addr_({0}), isClose_(true){}
HttpConn::~HttpConn(){
    Close();
}

void HttpConn::Init(int fd, const sockaddr_in &addr){
    userCount++;
    addr_ = addr;
    fd_ = fd;
    isClose_ = false;
    request_.Init(fd);
    response_.fd_ = fd;
}

void HttpConn::Close(){
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
    }
}

// 0:解析正确 1:继续监听 2:关闭连接 3:重定向 else:文件未找到
int HttpConn::read_process(){
    int ret = request_.process();
    printf("解析完成:%d\n", ret);
    std::cout << "请求资源:" << request_.Resource << std::endl;

    if(ret == 0){
        response_.Init(srcDir_, request_.Resource, request_.IsKeepAlice(), 200);
    }
    else if(ret == 1 || ret == 2)
        return ret;
    else if(ret == 3){
        response_.Init(srcDir_, "/public", request_.IsKeepAlice(), 302);
    }
    else
        response_.Init(srcDir_, request_.Resource, false, 400);

    request_.Init(fd_);
    request_.RecvMsg = "";

    return ret;
}

// 0:发送完成 1:继续发送 2:关闭连接
int HttpConn::write_process(){
    int ret = response_.process();
    if(ret == 0){
        if(response_.IsKeepAlice())
            return 0;
        else
            return 2;
    }
    return ret;
}

int HttpConn::GetFd() const{
    return fd_;
}