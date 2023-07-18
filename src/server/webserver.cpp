#include "webserver.h"

WebServer::WebServer(int port){
    HttpConn::srcDir_ = "../resources";

    listenEvent_ = EPOLLRDHUP | EPOLLET;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;

    int ret;
    struct linger optLinger = {0};
    struct sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(port);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenFd_ >= 0);
    // error

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    // error

    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    // error

    ret = bind(listenFd_, (sockaddr*)&listenAddr, sizeof(listenAddr));
    assert(ret >= 0);
    // error

    ret = listen(listenFd_, 10);
    assert(ret >= 0);
    // error

    setNonBlocking(listenFd_);

    globalEpoll().addFd(listenFd_, listenEvent_ | EPOLLIN);
}

WebServer::~WebServer(){
    close(listenFd_);
}

void WebServer::startUp(){
    struct epoll_event events[1024];
    while(true){
        int eventCnt = epoll_wait(globalEpoll().GetFd(), events, 1024, -1);
        for(int i = 0; i < eventCnt; i ++){
            int fd = events[i].data.fd;
            uint32_t event = events[i].events;
            if(fd == listenFd_){
                dealNew_();
            }
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                globalEpoll().delFd(fd);
                close(fd);
            }
            else if(event & EPOLLIN){
                dealRead_(&users_[fd]);
            }
            else if(event & EPOLLOUT){
                dealWrite_(&users_[fd]);
            }
            else{
                // error
            }
        }
    }
}

void WebServer::dealNew_(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd = accept(listenFd_, (sockaddr*)&addr, &len);
    if(fd <= 0) return;
    globalEpoll().addFd(fd, connEvent_ | EPOLLIN);
    setNonBlocking(fd);
    users_[fd].Init(fd, addr);
}

// 分发工作
void WebServer::dealRead_(HttpConn *client){
    globalThreadPool().AddTask(std::bind(&WebServer::OnRead_, this, client));
}
void WebServer::dealWrite_(HttpConn *client){
    globalThreadPool().AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::OnRead_(HttpConn *client){
    // 读取
    client->Read();

    // 解析
    if(client->parse() == false)
        return;

    // 准备发送
    client->process();
    globalEpoll().modFd(client->GetFd(), connEvent_ | EPOLLOUT);
}
void WebServer::OnWrite_(HttpConn *client){
    // 发送
    client->Send();
    // globalEpoll().modFd(client->GetFd(), connEvent_ | EPOLLIN);
    globalEpoll().delFd(client->GetFd());
    close(client->GetFd());
    printf("发送完成！\n");
}