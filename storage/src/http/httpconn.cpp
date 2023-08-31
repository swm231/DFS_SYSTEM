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

void HttpConn::Init(int fd_, const sockaddr_in &addr){
    userCount++;
    addr_ = addr;
    fd = fd_;
    Message_->fd_ = fd_;
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
        Message_->fd_ = -1;
    }
}
bool HttpConn::isClose(){
    return isClose_;
}

// 0:解析正确 1:继续监听 2:关闭连接
int HttpConn::ReadProcess(){
    int ret = request_.process();
    if(ret == 0){
        UpdateSql_();
        response_.Init();
    }
    else
        return ret;
    return ret;
}

// 0:发送完成 1:继续发送 2:关闭连接
int HttpConn::WriteProcess(){
    int ret = response_.process();
    if(ret == 0){
        request_.Init();
        return 2;
    }
    return ret;
}

int HttpConn::GetFd() const{
    if(isClose_)
        return -1;
    return Message_->fd_;
}

void HttpConn::UpdateSql_(){
    if(Message_->Behavior == BEHAVIOR::DOWNLOAD || Message_->Behavior == BEHAVIOR::BEHAVIOR_OTHER)
        return;
    MYSQL *sql;
    SqlConnRAII RAII(&sql);
    char order[256] = {0};
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row;
    if(Message_->Behavior == BEHAVIOR::UPLOAD){
        if(Message_->Path == PATH::PUBLIC)
            snprintf(order, 256, "INSERT INTO public values ('%s');", Message_->FileName.c_str());
        else if(Message_->Path == PATH::PRIVATE)
            snprintf(order, 256, "INSERT INTO public values ('%s', '%s');", Message_->UserName.c_str(), Message_->FileName.c_str());
    }
    else if(Message_->Behavior == BEHAVIOR::DELETE){
        if(Message_->Path == PATH::PUBLIC)
            snprintf(order, 256, "DELETE FROM public WHERE filename = '%s';", Message_->FileName.c_str());
        else if(Message_->Path == PATH::PRIVATE)
            snprintf(order, 256, "DELETE FROM private WHERE username = '%s' AND filename = '%s';", Message_->UserName.c_str(), Message_->FileName.c_str());
    }
    if(mysql_query(sql, order))
        LOG_ERROR("[sql] updatesql error");

    UpdateGroup_();
}

void HttpConn::UpdateGroup_(){
    for(auto it: StorageNode::group){
        it.second->ReadySYN(Message_->Behavior, Message_->Path, Message_->UserName, Message_->FileName);
        Epoll::Instance().modFd(it.second->fd, it.second, Epoll::connEvent_ | EPOLLOUT);
    }
}