#include "trackerserver.h"

std::atomic<int> ThreadPool::free_;

TrackerServer::TrackerServer(){
    
    stop_ = false;
    Epoll::listenEvent_ = EPOLLRDHUP;
    Epoll::connEvent_ = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;

    Log::Instance().Init(0);

    Conf::Instance().Parse();
    HeapTimer::Instance().Init();

    SqlConnPool::Instance().Init(Conf::mysql_host.c_str(), 
        Conf::mysql_user.c_str(), Conf::mysql_pwd.c_str(), Conf::mysql_database.c_str());

    listenhttp_ = new ListenHttp(Conf::http_port);
    Epoll::Instance().addFd(listenhttp_->fd, listenhttp_, Epoll::connEvent_ | EPOLLIN);
    listentask_ = new ListenTask(Conf::task_port);
    Epoll::Instance().addFd(listentask_->fd, listentask_, Epoll::connEvent_ | EPOLLIN);

}
TrackerServer::~TrackerServer(){

}

void TrackerServer::StartUp(){
    struct epoll_event events[1024];
    int timeMS = -1;
    while(stop_ == false){
        if(Conf::timeOut > 0)
            timeMS = HeapTimer::Instance().GetNextTick();
        int eventCnt = epoll_wait(Epoll::Instance().GetFd(), events, 1024, timeMS);
        for(int i = 0; i < eventCnt; i++){
            uint32_t event = events[i].events;
            if(event & EPOLLIN)
                DealRead_(static_cast<BaseNode*>(events[i].data.ptr));
            else if(event & EPOLLOUT)
                DealWrite_(static_cast<BaseNode*>(events[i].data.ptr));
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
                DealClose_(static_cast<BaseNode*>(events[i].data.ptr));
            else
                LOG_ERROR("[epoll] Unexpected Event!");
        }
    }
}
void TrackerServer::CloseServer(int signum){
    stop_ = true;
}

void TrackerServer::DealRead_(BaseNode *ptr){
    ThreadPool::Instance().AddTask(std::bind(&TrackerServer::OnRead_, this, ptr));
}
void TrackerServer::DealWrite_(BaseNode *ptr){
    ThreadPool::Instance().AddTask(std::bind(&TrackerServer::OnWrite_, this, ptr));
}

void TrackerServer::OnRead_(BaseNode *ptr){
    int ret = ptr->ReadProcess();
    if(ret == 0)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLOUT);
    else if(ret == 1)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLIN);
    else if(ret == 2)
        DealClose_(ptr);
}
void TrackerServer::OnWrite_(BaseNode *ptr){
    int ret = ptr->WriteProcess();
    if(ret == 0)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLIN);
    else if(ret == 1)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLOUT);
    else if(ret == 2)
        DealClose_(ptr);
}
void TrackerServer::DealClose_(BaseNode *ptr){
    Epoll::Instance().delFd(ptr->fd);
    delete ptr;
}