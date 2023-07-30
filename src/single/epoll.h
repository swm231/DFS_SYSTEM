#pragma once

#include <iostream>
#include <sys/epoll.h>
#include <sys/fcntl.h>

class Epoll{
public:
    static Epoll &Instance(){
        static Epoll instance;
        return instance;
    }

    int addFd(int fd, uint32_t events);
    int modFd(int fd, uint32_t events);
    int delFd(int fd);
    
    int GetFd() const{
        return epfd_;
    }

private:
    Epoll(int maxEvent = 512);
    ~Epoll();
    
    int epfd_;
};

void setNonBlocking(int fd);