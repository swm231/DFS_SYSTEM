#pragma once

#include <iostream>
#include <sys/epoll.h>
#include <sys/fcntl.h>

class Epoll{
public:
    Epoll(int maxEvent = 512);
    ~Epoll();
    
    int addFd(int fd, uint32_t events);
    int modFd(int fd, uint32_t events);
    int delFd(int fd);
    
    int GetFd() const{
        return epfd_;
    }

private:
    int epfd_;
};

void setNonBlocking(int fd);