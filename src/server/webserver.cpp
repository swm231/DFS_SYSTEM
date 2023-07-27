#include "webserver.h"

int Response::CookieOut;
char Message::CR[] = "\r";
char Message::CRLF[] = "\r\n";
WebServer::WebServer(int port, int timeoutMS, const char *host, const char *username, const char *pwd,
        const char *dbname, bool OpenLog, int Loglevel, int cookieOut)
         : port_(port), timeoutMS_(timeoutMS), stop_(false){
    HttpConn::srcDir_ = "../resources/";

    if(OpenLog)
        globalLog().Init(Loglevel);

    Response::CookieOut = cookieOut;
    srand(time(NULL));

    listenEvent_ = EPOLLRDHUP | EPOLLET;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP | EPOLLET;

    if(InitListenSocket() == false){
        LOG_ERROR("套接字初始化失败！服务器启动失败！");
        stop_ = true;
    }
    globalSqlConnPool().Init(host, username, pwd, dbname);

    if(!stop_)
        LOG_INFO("============SERVER START============");
}
WebServer::~WebServer(){
    close(listenFd_);
}

void WebServer::startUp(){
    int timeMS = -1;
    struct epoll_event events[1024];
    while(!stop_){
        if(timeoutMS_ > 0)
            timeMS = globalHeapTimer().GetNextTick();
        int eventCnt = epoll_wait(globalEpoll().GetFd(), events, 1024, timeMS);
        for(int i = 0; i < eventCnt; i ++){
            int fd = events[i].data.fd;
            uint32_t event = events[i].events;
            if(fd == listenFd_){
                dealNew_();
            }
            else if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                closeConn_(&users_[fd]);
            }
            else if(event & EPOLLIN){
                dealRead_(&users_[fd]);
            }
            else if(event & EPOLLOUT){
                dealWrite_(&users_[fd]);
            }
            else{
                LOG_ERROR("[epoll] Unexpected Event!");
            }
        }
    }
}

void WebServer::dealNew_(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd = accept(listenFd_, (sockaddr*)&addr, &len);
    if(fd <= 0) return;
    users_[fd].Init(fd, addr);
    if(timeoutMS_ > 0)
        globalHeapTimer().add(fd, timeoutMS_, std::bind(&WebServer::closeConn_, this, &users_[fd]));
    globalEpoll().addFd(fd, connEvent_ | EPOLLIN);
    setNonBlocking(fd);

}
void WebServer::closeConn_(HttpConn *client){
    globalEpoll().delFd(client->GetFd());
    client->Close();

}

// 分发工作
void WebServer::dealRead_(HttpConn *client){
    globalThreadPool().AddTask(std::bind(&WebServer::OnRead_, this, client));
    updateTimer_(client);
}
void WebServer::dealWrite_(HttpConn *client){
    globalThreadPool().AddTask(std::bind(&WebServer::OnWrite_, this, client));
    updateTimer_(client);
}

// 0:解析正确 1:继续监听 2:关闭连接
void WebServer::OnRead_(HttpConn *client){
    int ret = client->read_process();
    if(ret == 1)
        globalEpoll().modFd(client->GetFd(), connEvent_ | EPOLLIN);
    else if(ret == 2)
        closeConn_(client);
    else
        globalEpoll().modFd(client->GetFd(), connEvent_ | EPOLLOUT);
}
// 0:发送完成 1:继续发送 2:关闭连接
void WebServer::OnWrite_(HttpConn *client){
    int ret = client->write_process();
    if(ret == 0){
        globalEpoll().modFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
    else if(ret == 1)
        globalEpoll().modFd(client->GetFd(), connEvent_ | EPOLLOUT);
    else
        closeConn_(client);
}

void WebServer::updateTimer_(HttpConn *client){
    if(timeoutMS_ > 0)
        globalHeapTimer().update(client->GetFd(), timeoutMS_);
}

bool WebServer::InitListenSocket(){
    int ret;
    struct linger optLinger;

    struct sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(port_);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ == -1){
        LOG_ERROR("[socket] init ERROR");
        return false;
    }

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret == -1){
        LOG_ERROR("[socket] set ERROR");
        return false;
    }

    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1){
        LOG_ERROR("[socket] set ERROR");
        return false;
    }

    ret = bind(listenFd_, (sockaddr*)&listenAddr, sizeof(listenAddr));
    if(ret == -1){
        LOG_ERROR("[socket] bind ERROR");
        return false;
    }

    ret = listen(listenFd_, 10);
    if(ret == -1){
        LOG_ERROR("[socket] listen ERROR");
        return false;
    }

    setNonBlocking(listenFd_);
    globalEpoll().addFd(listenFd_, listenEvent_ | EPOLLIN);

    return true;
}