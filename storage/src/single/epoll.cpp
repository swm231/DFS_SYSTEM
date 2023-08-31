#include "epoll.h"

uint32_t Epoll::listenEvent_, Epoll::connEvent_;
Epoll::Epoll(int maxEvent) : epfd_(epoll_create(maxEvent)) {}

Epoll::~Epoll() {}

int Epoll::addFd(int fd, FdNode *ptr, uint32_t events){
    epoll_event event;
    bzero(&event, sizeof event);
    event.data.ptr = ptr;
    event.events = events;

    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
    if(ret < 0)
        LOG_ERROR("[epoll] EPOLL_CTL_ADD ERROR! errno:%d", errno);
    return 0;
}

int Epoll::modFd(int fd, FdNode *ptr, uint32_t events){
    epoll_event event = {0};
    event.data.ptr = ptr;
    event.events = events;

    int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
    if(ret < 0)
        LOG_ERROR("[epoll] EPOLL_CTL_MOD ERROR! errno:%d", errno);
    return 0;
}

int Epoll::delFd(int fd){
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    if(ret < 0){
        LOG_ERROR("[epoll] EPOLL_CTL_DEL ERROR! errno:%d", errno);
    }
    return 0;
}

void setNonBlocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}