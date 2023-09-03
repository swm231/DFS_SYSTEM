#pragma once

#include "config.h"

class SyncTracker{
public:
    static void SyncSelf(){
        for(int i = 0; i < Conf::Instance().tracker.size(); i++){
            Conf::Instance().tracker[i]->ReadySynSelf();
            Epoll::Instance().modFd(Conf::Instance().tracker[i]->fd,
                Conf::Instance().tracker[i], Epoll::connEvent_ | EPOLLOUT);
        }
    }

    static void SyncUser(const std::string &username){
        for(int i = 0; i < Conf::Instance().tracker.size(); i++){
            Conf::Instance().tracker[i]->ReadySynUser(username);
            Epoll::Instance().modFd(Conf::Instance().tracker[i]->fd,
                Conf::Instance().tracker[i], Epoll::connEvent_ | EPOLLOUT);
        }
    }
};