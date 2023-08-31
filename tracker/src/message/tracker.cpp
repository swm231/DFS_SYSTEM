#include "tracker.h"
#include "../http/httpconn.h"


char Message::CR[] = "\r";
char Message::CRLF[] = "\r\n";
std::unordered_map<int, StorageNode*> Message::storageNode;
std::unordered_map<std::string, std::list<int> > Message::group;

FdNode::FdNode():ip(0), port(0), fd(0), saveErrno(0), status(MSGSTATUS::HANDLE_INIT), isOut(false){}
FdNode::FdNode(uint32_t _ip, uint16_t _port, uint16_t _fd): ip(_ip), port(_port), fd(_fd), 
    saveErrno(0), status(MSGSTATUS::HANDLE_INIT), isOut(false){}
FdNode::FdNode(FdNode &&other):ip(std::move(other.ip)), port(std::move(other.port)), fd(std::move(other.fd)),
    saveErrno(std::move(other.saveErrno)), recvBuff(std::move(other.recvBuff)), sendBuff(std::move(other.sendBuff)),
    status(std::move(other.status)), isOut(std::move(other.isOut)){}
FdNode::~FdNode(){
    if(fd != 0){
        close(fd);
        fd = 0;
    }
}
void FdNode::Init(){
    isOut = false;
    status = MSGSTATUS::HANDLE_INIT;
}
int FdNode::ReadProcess(){
    while(true){
        ssize_t recvSize = recvBuff.ReadFd(fd, &saveErrno);
        if(recvSize == 0)
            return 2;
        if(recvSize == -1){
            if(saveErrno != EAGAIN)
                return 2;
            return 1;
        }

        if(status == HANDLE_INIT)
            ParseHead_();
        if(status == HANDLE_BODY)
            ParseBody_();
        if(status == HANDLE_COMPLATE){
            Init();
            if(isOut)
                return 0;
            else
                return 1;
        }
        if(status == HANDLE_ERROR)
            return 2;
    }
    return 0;
}
int FdNode::WriteProcess(){
    ssize_t sendSize = 0;
    sendSize = sendBuff.WriteFd(static_cast<int>(fd), &saveErrno);
    if(sendSize == 0)
        return 2;
    if(sendSize < 0){
        if(saveErrno == EAGAIN)
            return 1;
        return 2;
    }
    if(sendBuff.UnHandleBytes())
        return 1;
    else return 0;
}


StorageNode::StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd): FdNode(_ip, _port, _fd){}
StorageNode::StorageNode(StorageNode &&other): FdNode(std::move(other)){
    TaskPort = std::move(other.TaskPort);  HttpPort = std::move(other.HttpPort);
    TotalSize = std::move(other.TotalSize);  UsedSize = std::move(other.UsedSize);
    Status = std::move(other.Status);  groupName = std::move(other.groupName);
}
StorageNode::~StorageNode(){
    std::list<int>::iterator pos;
    bool flag = false;
    for(std::list<int>::iterator it = Message::group[groupName].begin(); 
        it != Message::group[groupName].end(); it++)
            if(*it == fd){
                pos = it;
                flag = true;
                break;
            }
    if(flag)
        Message::group[groupName].erase(pos);
    if(Message::storageNode.count(fd))
        Message::storageNode.erase(fd);
}
void StorageNode::ParseHead_(){
    const char *lineEnd = std::search(recvBuff.Peek(), recvBuff.BeginWriteConst(), Message::CRLF, Message::CRLF + 2);
    if(lineEnd == recvBuff.BeginWriteConst())
        return;

    std::string str(recvBuff.Peek(), 3);
    recvBuff.AddHandled(4);
    if(str == "NEW")
        DealNEW_(lineEnd);
    else if(str == "KEP")
        DealKEP_(lineEnd);
}
void StorageNode::DealNEW_(const char *lineEnd){
    Status = STORAGE_STATUS_WAIT_SYNC;
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);

    std::istringstream stream(byteStreamString);
    stream >> groupName >> TaskPort >> HttpPort >> TotalSize >> UsedSize;

    // 准备当前分组的服务器信息
    sendBuff.Append("new");
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Message::group[groupName].size()));
    sendBuff.Append(" ");
    for(auto it:Message::group[groupName]){
        sendBuff.Append(std::to_string(Message::storageNode[it]->ip));
        sendBuff.Append(" ");
        sendBuff.Append(std::to_string(Message::storageNode[it]->TaskPort));
        sendBuff.Append(" ");
    }
    sendBuff.Append("\r\n");

    if(Message::group[groupName].size() == 0 && groupName != "public")
        ConsistentHash::Instance().AddNode(groupName);

    Message::group[groupName].push_back(fd);
    isOut = true;
    status = HANDLE_COMPLATE;
}
void StorageNode::DealKEP_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
    sendBuff.Append("kep");
    sendBuff.Append("\r\n");
    isOut = true;
    status = HANDLE_COMPLATE;
}
void StorageNode::SignalAll_(int signum){
    for(auto it : Message::group[groupName])
        Message::storageNode[it]->Status = signum;
}


int ListenHttp::ReadProcess(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd_ = accept(fd, (sockaddr*)&addr, &len);
    if(fd_ <= 0) return 1;
    HttpConn *ptr = new HttpConn();
    ptr->Init(fd_, addr);
    setNonBlocking(fd_);
    Epoll::Instance().addFd(fd_, ptr, Epoll::connEvent_ | EPOLLIN);
    return 1;
}
int ListenHttp::WriteProcess(){ return 0; }


int ListenTask::ReadProcess(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd_ = accept(fd, (sockaddr*)&addr, &len);
    if(fd_ <= 0) return 1;
    StorageNode *ptr = new StorageNode(ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port), static_cast<uint16_t>(fd_));
    Message::storageNode[fd_] = ptr;
    setNonBlocking(fd_);
    Epoll::Instance().addFd(fd_, ptr, Epoll::connEvent_ | EPOLLIN);
    return 1;
}
int ListenTask::WriteProcess(){ return 0; }


bool FdNode::InitListenSocket(uint16_t &sockt, int port){
    int ret;
    struct sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(port);

    sockt = socket(AF_INET, SOCK_STREAM, 0);
    if(sockt == -1){
        LOG_ERROR("[socket] init ERROR");
        return false;
    }

    int optval = 1;
    ret = setsockopt(sockt, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1){
        LOG_ERROR("[socket] set ERROR");
        return false;
    }

    ret = bind(sockt, (sockaddr*)&listenAddr, sizeof(listenAddr));
    if(ret == -1){
        LOG_ERROR("[socket] bind ERROR");
        return false;
    }

    ret = listen(sockt, 4096);
    if(ret == -1){
        LOG_ERROR("[socket] listen ERROR");
        return false;
    }

    setNonBlocking(sockt);

    return true;
}
