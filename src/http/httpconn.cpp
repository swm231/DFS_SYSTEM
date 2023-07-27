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

    LOG_INFO("[http] 新的连接 ip:%s port:%d fd:%d 在线人数:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), fd, (int)userCount);
}

void HttpConn::Close(){
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
        request_.Close();
        response_.Close();
        
        LOG_INFO("[http] 连接关闭 fd:%d 在线人数:%d", fd_, (int)userCount);
    }
}

// 0:解析正确 1:继续监听 2:关闭连接
int HttpConn::read_process(){
    int ret = request_.process();
    if(ret == 0)
        response_.Init(srcDir_, request_.Get_resDir(), request_.Get_action(), request_.Resource, request_.Get_username(), request_.isSetCookie(), 
            request_.Get_cookie(), request_.IsKeepAlice(), request_.Get_code() == -1 ? 200 : request_.Get_code());
    else
        return ret;

    request_.Init(fd_);
    request_.RecvMsg.AddHandledAll();

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