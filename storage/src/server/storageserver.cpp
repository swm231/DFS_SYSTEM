#include "storageserver.h"


StorageServer::StorageServer(){
    Epoll::listenEvent_ = EPOLLRDHUP;
    Epoll::connEvent_ = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;

    stop_ = false;
    Log::Instance().Init(1);

    if(Conf::Instance().Parse() == false){
        LOG_ERROR("[conf] 文件解析错误");
        stop_ = true;
    }

    SqlConnPool::Instance().Init(Conf::Instance().mysql_host.c_str(), Conf::Instance().mysql_user.c_str(), 
        Conf::Instance().mysql_pwd.c_str(), Conf::Instance().mysql_database.c_str());

    listenhttp_ = std::make_unique<ListenHttp>(Conf::Instance().http_port);
    Epoll::Instance().addFd(listenhttp_->fd, listenhttp_.get(), Epoll::listenEvent_ | EPOLLIN);
    listentask_ = std::make_unique<ListenTask>(Conf::Instance().task_port);
    Epoll::Instance().addFd(listentask_->fd, listentask_.get(), Epoll::listenEvent_ | EPOLLIN);

    Connect_();

    ThrobThread = std::make_unique<std::thread>(&StorageServer::KeepThrob_, this);
}
StorageServer::~StorageServer(){ 
    if(ThrobThread->joinable())
        ThrobThread->join();
    for(auto it: Conf::Instance().tracker){
        Epoll::Instance().delFd(it->fd);
        delete it;
    }
}

void StorageServer::StartUp(){
    struct epoll_event events[1024];
    while(stop_ == false){
        int eventCnt = epoll_wait(Epoll::Instance().GetFd(), events, 1024, -1);
        for(int i = 0; i < eventCnt; i++){
            uint32_t event = events[i].events;
            if(event & EPOLLIN)
                DealRead_(static_cast<FdNode*>(events[i].data.ptr));
            else if(event & EPOLLOUT)
                DealWrite_(static_cast<FdNode*>(events[i].data.ptr));
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
                DealClose_(static_cast<FdNode*>(events[i].data.ptr));
            else
                LOG_ERROR("[epoll] Unexpected Event!");
        }
    }
}
void StorageServer::CloseServer(int signum){
    std::this_thread::sleep_for(std::chrono::seconds(2));
    StorageServer::stop_ = true;
}

void StorageServer::DealRead_(FdNode *ptr){
    ThreadPool::Instance().AddTask(std::bind(&StorageServer::OnRead_, this, ptr));
}
void StorageServer::DealWrite_(FdNode *ptr){
    ThreadPool::Instance().AddTask(std::bind(&StorageServer::OnWrite_, this, ptr));
}
void StorageServer::OnRead_(FdNode *ptr){
    int ret = ptr->ReadProcess();
    if(ret == 0)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLOUT);
    else if(ret == 1)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLIN);
    else if(ret == 2)
        DealClose_(ptr);
}
void StorageServer::OnWrite_(FdNode *ptr){
    int ret = ptr->WriteProcess();
    if(ret == 0)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLIN);
    else if(ret == 1)
        Epoll::Instance().modFd(ptr->fd, ptr, Epoll::connEvent_ | EPOLLOUT);
    else if(ret == 2)
        DealClose_(ptr);
}
void StorageServer::DealClose_(FdNode *ptr){
    Epoll::Instance().delFd(ptr->fd);
    delete ptr;
}

void StorageServer::Connect_(){
    struct sockaddr_in addr;
    for(int i = 0; i < Conf::Instance().tracker.size(); i++){
        Conf::Instance().tracker[i]->fd = socket(AF_INET, SOCK_STREAM, 0);

        if(Conf::Instance().tracker[i]->fd < 0){
            LOG_ERROR("[socket] tracker socket init error");
            continue;
        }
        memset(&addr, 0, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(Conf::Instance().tracker[i]->ip);
        addr.sin_port = htons(Conf::Instance().tracker[i]->port);
        if(connect(Conf::Instance().tracker[i]->fd, (sockaddr*)&addr, sizeof addr) < 0){
            LOG_ERROR("[socket] tracker socket connect error");
            continue;
        }

        Epoll::Instance().addFd(Conf::Instance().tracker[i]->fd, Conf::Instance().tracker[i], Epoll::connEvent_);

        Conf::Instance().tracker[i]->ReadyTraNEW();
        Epoll::Instance().modFd(Conf::Instance().tracker[i]->fd, Conf::Instance().tracker[i], Epoll::connEvent_ | EPOLLOUT);
    }
}

void StorageServer::KeepThrob_(){
    // TODO
    // while(true){
    //     std::this_thread::sleep_for(std::chrono::seconds(2));
    //     if(stop_)
    //         break;
    //     for(int i = 0; i < Conf::Instance().tracker.size(); i++){
    //         Conf::Instance().tracker[i].ReadyTraKEP();
    //         // Epoll::Instance().modFd(Conf::Instance().tracker[i].fd, &(Conf::Instance().tracker[i]), Epoll::connEvent_ | EPOLLOUT);
    //     }
    // }
}