#include "httpconn.h"
extern Epoll& globalEpoll();

std::atomic<int> HttpConn::userCount;
std::unordered_set<std::string> HttpMessage::onlieUser; 
std::unordered_map<std::string, std::unordered_map<int, int> > HttpMessage::userServer;

HttpConn::HttpConn() : addr_({0}), isClose_(true), Message_(new HttpMessage()), 
    request_(Message_), response_(Message_){}
HttpConn::~HttpConn(){
    if(Message_->UserName != "")
        HttpMessage::onlieUser.erase(Message_->UserName);
    HeapTimer::Instance().erase(fd);
    Close();
    delete Message_;
}

void HttpConn::Init(int _fd, const sockaddr_in &addr){
    userCount++;
    addr_ = addr;
    fd = _fd;
    Message_->fd_ = _fd;
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
    HeapTimer::Instance().update(fd, Conf::timeOut);
    int ret = request_.process();
    if(request_.GetLogStatus() == 0 && HttpMessage::userServer.count(Message_->UserName) == 0)
        addUserServer();
    if(ret == 0)
        response_.Init();
    else
        return ret;
    return ret;
}

// 0:发送完成 1:继续发送 2:关闭连接
int HttpConn::WriteProcess(){
    HeapTimer::Instance().update(fd, Conf::timeOut);
    int ret = response_.process();
    if(ret == 0){
        if(request_.IsKeepAlice()){
            request_.Init();
            return 0;
        }
        else
            return 2;
    }
    return ret;
}

void HttpConn::addUserServer(){
    HttpMessage::onlieUser.insert(Message_->UserName);
    // 用户信息还没同步完，因此信息没有删除。此时登录不需要添加服务器信息
    if(HttpMessage::userServer.count(Message_->UserName) != 0)
        return;
    std::string group_ = ConsistentHash::Instance().GetGroup(Message_->UserName);
    // 上读锁
    ReaderLockRAII locker(Message::lock);
    for(auto it: Message::group[group_])
        // 添加用户相关分组服务器信息；
        HttpMessage::userServer[Message_->UserName][it] =
            Message::storageNode.find(it)->second->Status;
}

