#include "../node.h"
#include "../../http/httpconn.h"

int ListenHttp::ReadProcess(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd_ = accept(fd, (sockaddr*)&addr, &len);
    if(fd_ <= 0) return 1;
    HttpConn *ptr = new HttpConn();
    ptr->Init(fd_, addr);
    HeapTimer::Instance().add(fd_, Conf::timeOut, std::bind(&ListenHttp::CloseUser, this, ptr));
    setNonBlocking(fd_);
    Epoll::Instance().addFd(fd_, ptr, Epoll::connEvent_ | EPOLLIN);
    return 1;
}
int ListenHttp::WriteProcess(){ return 0; }
void ListenHttp::CloseUser(BaseNode *ptr){
    Epoll::Instance().delFd(ptr->fd);
    delete ptr;
}


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
