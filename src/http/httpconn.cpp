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
}

void HttpConn::Close(){
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
    }
}


// 0:解析正确 1:继续监听 2:关闭连接 3:重定向 else:文件未找到
int HttpConn::process(){
    int ret = request_.parse();
    printf("解析完成:%d\n", ret);

    if(ret == 0){
        response_.Init(srcDir_, request_.Resource, request_.IsKeepAlice(), 200);
    }
    else if(ret == 1 || ret == 2)
        return ret;
    else if(ret == 3){
        response_.Init(srcDir_, "/public.html", request_.IsKeepAlice(), 302);
    }
    else
        response_.Init(srcDir_, request_.Resource, false, 400);

    request_.Init(fd_);
    request_.RecvMsg = "";
    response_.MaskeResponse();

    return ret;
}

int HttpConn::Send(int *writeErrno){
    if(response_.HeadStatus == HANDLE_COMPLATE)
        return -1;
    while(1){
        unsigned long long sentLen = 0;
        
        // 头部
        if(response_.HeadStatus == HANDLE_HEAD){
            sentLen = response_.HasSendLen;
            sentLen = send(fd_, response_.beforeBodyMsg.c_str() + sentLen, response_.beforeBodyMsgLen - sentLen, 0);
            if(sentLen == -1){
                // 如果不是缓冲区满了，就是出错了
                *writeErrno = errno;
                break;
            }
            response_.HasSendLen += sentLen;
            if(response_.HasSendLen >= response_.beforeBodyMsgLen){
                response_.HeadStatus = HANDLE_BODY;
                response_.HasSendLen = 0;
            }
        }

        // 消息体
        if(response_.HeadStatus == HANDLE_BODY){
            sentLen = response_.HasSendLen;
            sentLen = send(fd_, response_.MsgBody.c_str() + sentLen, response_.MsgBodyLen - sentLen, 0);
            if(sentLen == -1){
                // 如果不是缓冲区满了，就是出错了
                *writeErrno = errno;
                break;
            }
            response_.HasSendLen += sentLen;
            if(response_.HasSendLen >= response_.MsgBodyLen){
                response_.HeadStatus = HANDLE_COMPLATE;
                response_.HasSendLen = 0;
                break;
            }
        }
    }

}

int HttpConn::GetFd() const{
    return fd_;
}