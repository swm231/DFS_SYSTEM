#pragma once

#include <iostream>
#include <sys/epoll.h>
#include <sys/fcntl.h>

#include "../log/log.h"

class FdNode;
class Epoll{
public:
    static Epoll &Instance(){
        static Epoll instance;
        return instance;
    }

    int addFd(int fd, FdNode *ptr, uint32_t events);
    int modFd(int fd, FdNode *ptr, uint32_t events);
    int delFd(int fd);
    
    int GetFd() const{
        return epfd_;
    }

    static uint32_t listenEvent_;
    static uint32_t connEvent_;
private:
    Epoll(int maxEvent = 512);
    ~Epoll();
    
    int epfd_;
};

void setNonBlocking(int fd);