#include "httpconn.h"
extern Epoll& globalEpoll();

std::atomic<int> HttpConn::userCount;
std::string HttpConn::srcDir_;

HttpConn::HttpConn() : addr_({0}), isClose_(true), Message_(new HttpMessage()), 
    request_(Message_), response_(Message_){}
HttpConn::~HttpConn(){
    Close();
    delete Message_;
}

void HttpConn::Init(int fd, const sockaddr_in &addr){
    userCount++;
    addr_ = addr;
    Message_->fd_ = fd;
    isClose_ = false;
    request_.Init();

    LOG_INFO("[http] 新的连接 ip:%s port:%d fd:%d 在线人数:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), fd, (int)userCount);
}

void HttpConn::Close(){
    if(isClose_ == false){
        LOG_INFO("[http] 连接关闭 fd:%d 在线人数:%d", Message_->fd_, (int)userCount);

        isClose_ = true;
        userCount--;
        request_.Close();
        response_.Close();
        close(Message_->fd_);
    }
}
bool HttpConn::isClose(){
    return isClose_;
}

// 0:解析正确 1:继续监听 2:关闭连接
int HttpConn::read_process(){
    int ret = request_.process();
    if(ret == 0)
        response_.Init();
    else
        return ret;
    return ret;
}

// 0:发送完成 1:继续发送 2:关闭连接
int HttpConn::write_process(){
    int ret = response_.process();
    if(ret == 0){
        request_.Init();
        if(request_.IsKeepAlice())
            return 0;
        else
            return 2;
    }
    return ret;
}

int HttpConn::GetFd() const{
    if(isClose_)
        return -1;
    return Message_->fd_;
}