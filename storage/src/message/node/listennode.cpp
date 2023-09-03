#include "../node.h"
#include "../../single/epoll.h"
#include "../../http/httpconn.h"

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
    TaskNode *ptr = new TaskNode(ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port), fd_);
    setNonBlocking(fd_);
    Epoll::Instance().addFd(fd_, ptr, Epoll::connEvent_ | EPOLLIN);
    return 1;
}
int ListenTask::WriteProcess(){ return 0; }