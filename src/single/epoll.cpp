#include "epoll.h"


Epoll::Epoll(int maxEvent) : epfd_(epoll_create(maxEvent)) {}

Epoll::~Epoll() {}

int Epoll::addFd(int fd, uint32_t events){
    epoll_event event = {0};
    event.data.fd = fd;
    event.events = events;

    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
    if(ret < 0)
        LOG_ERROR("[epoll] EPOLL_CTL_ADD ERROR!");
    return 0;
}

int Epoll::modFd(int fd, uint32_t events){
    epoll_event event = {0};
    event.data.fd = fd;
    event.events = events;

    int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
    if(ret < 0)
        LOG_ERROR("[epoll] EPOLL_CTL_MOD ERROR!");
    return 0;
}

int Epoll::delFd(int fd){
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    if(ret < 0)
        LOG_ERROR("[epoll] EPOLL_CTL_DEL ERROR!");
    return 0;
}

void setNonBlocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}