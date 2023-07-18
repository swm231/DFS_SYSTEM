#include "epoll.h"


Epoll::Epoll(int maxEvent) : epfd_(epoll_create(maxEvent)) {}

Epoll::~Epoll() {}

int Epoll::addFd(int fd, uint32_t events){
    epoll_event event;
    event.data.fd = fd;
    event.events = events;

    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
    // error
    return 0;
}

int Epoll::modFd(int fd, uint32_t events){
    epoll_event event;
    event.data.fd = fd;
    event.events = events;

    int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
    // error
    return 0;
}

int Epoll::delFd(int fd){
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    // error
    return 0;
}

void setNonBlocking(int fd){
    // int ret = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}