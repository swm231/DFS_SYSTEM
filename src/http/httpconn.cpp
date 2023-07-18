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
}

void HttpConn::Close(){
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
    }
}

void HttpConn::Read(){
    char temp[2048];
    while(true){
        int len = recv(fd_, temp, 2028, 0);
        if(len <= 0) break;
        request_.Append(temp, len);
    }
    printf("读取完成！\n");
}

bool HttpConn::parse(){
    std::cout << "开始解析" << std::endl;
    std::cout << request_.RecvMsg << std::endl;

    std::string::size_type endIndex;

    // 请求行
    endIndex = request_.RecvMsg.find("\r\n");
    request_.setRequestLine(request_.RecvMsg.substr(0, endIndex + 2));
    request_.RecvMsg.erase(0, endIndex + 2);
    
    // 首部
    while(1){
        endIndex = request_.RecvMsg.find("\r\n");
        std::string curLine = request_.RecvMsg.substr(0, endIndex + 2);
        request_.RecvMsg.erase(0, endIndex + 2);
        if(curLine == "\r\n")
            break;
    }
    std::cout << "解析完成，请求资源:" << request_.Resource << std::endl;

    return true;
}

void HttpConn::process(){
    // 状态行
    response_.beforeBodyMsg = "HTTP/1.1 200 OK\r\n";

    // body
    std::ifstream fileStream("../resources/" + request_.Resource, std::ios::in);
    response_.MsgBody = "";
    std::string TempLine;
    while(getline(fileStream, TempLine)){
        response_.MsgBody += TempLine + "\n";
    }
    response_.MsgBodyLen = response_.MsgBody.size();

    // 头部
    if(response_.MsgBody != "")
        response_.beforeBodyMsg += "Content-Length: " + std::to_string(response_.MsgBodyLen) + "\r\n";
    response_.beforeBodyMsg += "Content-Type: " + GetFileType() + "\r\n";
    response_.beforeBodyMsg += "Connection: close\r\n";
    response_.beforeBodyMsg += "\r\n";
    response_.beforeBodyMsgLen = response_.beforeBodyMsg.size();

    response_.Status = HANDLE_HEAD;

    printf("数据准备完成！\n");
    std::cout << response_.beforeBodyMsg;
    std::cout << response_.MsgBody;
}

void HttpConn::Send(){
    if(response_.Status == HANDLE_COMPLATE)
        return;
    while(1){
        unsigned long long sentLen = 0;
        
        // 头部
        if(response_.Status == HANDLE_HEAD){
            sentLen = response_.HasSendLen;
            sentLen = send(fd_, response_.beforeBodyMsg.c_str() + sentLen, response_.beforeBodyMsgLen - sentLen, 0);
            if(sentLen == -1){
                // 如果不是缓冲区满了，就是出错了
                break;
            }
            response_.HasSendLen += sentLen;
            if(response_.HasSendLen >= response_.beforeBodyMsgLen){
                response_.Status = HANDLE_BODY;
                response_.HasSendLen = 0;
            }
        }

        // 消息体
        if(response_.Status == HANDLE_BODY){
            sentLen = response_.HasSendLen;
            sentLen = send(fd_, response_.MsgBody.c_str() + sentLen, response_.MsgBodyLen - sentLen, 0);
            if(sentLen == -1){
                // 如果不是缓冲区满了，就是出错了
                break;
            }
            response_.HasSendLen += sentLen;
            if(response_.HasSendLen >= response_.MsgBodyLen){
                response_.Status = HANDLE_COMPLATE;
                response_.HasSendLen = 0;
                break;
            }
        }
    }

}

std::string HttpConn::GetFileType(){
    std::string::size_type idx = request_.Resource.find_last_of('.');
    if(idx == std::string::npos)
        return "text/plain";
    std::string suffix = request_.Resource.substr(idx);
    if(response_.SUFFIX_TYPE.count(suffix))
        return response_.SUFFIX_TYPE.find(suffix)->second;
    return "text/plain";
}




int HttpConn::GetFd() const{
    return fd_;
}