#include "httpresponse.h"

const std::unordered_map<int, std::string> Response::CODE_STATUS = {
    { 200, "Ok"},
    { 302, "Moved Temporarily"},
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

HttpResponse::HttpResponse(HttpMessage *Message) : 
        fileMsgFd(-1), Message_(Message){}

HttpResponse::~HttpResponse(){}

void HttpResponse::Init(){
    HeadStatus = HANDLE_INIT;
    HasSentLen = BodyMsgLen = 0;
}
void HttpResponse::Close(){
    if(fileMsgFd != -1){
        close(fileMsgFd);
        fileMsgFd = -1;
    }
}

int HttpResponse::process(){
    if(HeadStatus == HANDLE_INIT){
        Parse_();

        AddStateLine_();

        AddHeader_();
        
        AddContent_();

        HeadStatus = HANDLE_HEAD;
        LOG_DEBUG("[http] fd:%d 数据准备完成", Message_->fd_);
    }

    while(true){
        long long sentLen = 0;
        if(HeadStatus == HANDLE_HEAD){
            Message_->SaveErrno = 0;
            sentLen = beforeBodyMsg.WriteFd(Message_->fd_, &(Message_->SaveErrno));
            if(sentLen == -1){
                if(Message_->SaveErrno != EAGAIN)
                    return 2;
                return 1;
            }

            if(beforeBodyMsg.UnHandleBytes() <= 0)
                HeadStatus = HANDLE_BODY;
        }

        if(HeadStatus == HANDLE_BODY){
            sentLen = HasSentLen;
            if(Message_->Behavior == BEHAVIOR::DOWNLOAD)
                sentLen = sendfile(Message_->fd_, fileMsgFd, (off_t*)&sentLen, BodyMsgLen - sentLen);
            if(sentLen == -1){
                if(errno != EAGAIN)
                    return 2;
                return 1;
            }
            HasSentLen += sentLen;

            if(HasSentLen >= BodyMsgLen){
                HeadStatus = HANDLE_COMPLATE;
                return 0;
            }
        }
    }
    return 2;
}

void HttpResponse::Parse_(){
    if(Message_->Behavior == BEHAVIOR::DOWNLOAD){
        if(Message_->Path == PATH::PUBLIC)
            fileMsgFd = open((Conf::Instance().data_path + "/public/" + Message_->FileName).c_str(), O_RDONLY);
        else if(Message_->Path == PATH::PRIVATE)
            fileMsgFd = open((Conf::Instance().data_path + "/rivate/" + Message_->UserName + "/" + Message_->FileName).c_str(), O_RDONLY);
        fstat(fileMsgFd, &fileSata_);
        return;
    }
    if(Message_->Behavior == BEHAVIOR::DELETE){
        if(Message_->Path == PATH::PUBLIC)
            remove((Conf::Instance().data_path + "/public/" + Message_->FileName).c_str());
        else if(Message_->Path == PATH::PRIVATE)
            remove((Conf::Instance().data_path + "/private/" + Message_->UserName + "/" + Message_->FileName).c_str());
        return;
    }
}

void HttpResponse::AddStateLine_(){
    // 状态行
    std::string status;
    if(CODE_STATUS.count(Message_->code))
        status = CODE_STATUS.find(Message_->code)->second;
    else{
        Message_->code = 400;
        status = CODE_STATUS.find(Message_->code)->second;
    }
    beforeBodyMsg.Append("HTTP/1.1 " + std::to_string(Message_->code) + " " + status + "\r\n");
}
void HttpResponse::AddHeader_(){
    // 头部
    if(Message_->Behavior == BEHAVIOR::DOWNLOAD)
        beforeBodyMsg.Append("Content-Type: application/octet-stream\r\n");
    beforeBodyMsg.Append("Connection: keep-alive\r\n");
    // beforeBodyMsg.Append("Refresh: 0;url=" + Message_->MsgHeader["Origin"] + "\r\n");
    // beforeBodyMsg.Append("Access-Control-Allow-Origin:" + Message_->MsgHeader["Origin"] + "\r\n");

    if(Message_->code == 302){
        if(Message_->Path == PATH::PUBLIC)
            beforeBodyMsg.Append("Location: " + Message_->MsgHeader["Origin"] + "/public\r\n");
        if(Message_->Path == PATH::PRIVATE)
            beforeBodyMsg.Append("Location: " + Message_->MsgHeader["Origin"] + "/private\r\n");
    }
}
void HttpResponse::AddContent_(){
    // body
    if(Message_->Behavior == BEHAVIOR::DOWNLOAD) {
        beforeBodyMsg.Append("Content-length: " + std::to_string(fileSata_.st_size) + "\r\n\r\n");
        beforeBodyMsgLen = beforeBodyMsg.UnHandleBytes();
        BodyMsgLen = fileSata_.st_size;
        return;
    }
}
std::string HttpResponse::GetTracker_(){
    if(Conf::Instance().tracker.size() == 0)
        return "";
    int rd = rand() % Conf::Instance().tracker.size();
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = Conf::Instance().tracker[rd]->ip;
    return inet_ntoa(addr.sin_addr) + std::string("/") + std::to_string(Conf::Instance().tracker[rd]->port);
}