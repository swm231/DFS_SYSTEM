#include "../basenode.h"
#include "../../log/log.h"
#include "../../single/epoll.h"

BaseNode::BaseNode():ip(0), port(0), fd(0), saveErrno(0), status(MSGSTATUS::HANDLE_INIT), isOut(false){}
BaseNode::BaseNode(uint32_t _ip, uint16_t _port, uint16_t _fd): ip(_ip), port(_port), fd(_fd), 
    saveErrno(0), status(MSGSTATUS::HANDLE_INIT), isOut(false){}
BaseNode::BaseNode(BaseNode &&other):ip(std::move(other.ip)), port(std::move(other.port)), fd(std::move(other.fd)),
    saveErrno(std::move(other.saveErrno)), recvBuff(std::move(other.recvBuff)), sendBuff(std::move(other.sendBuff)),
    status(std::move(other.status)), isOut(std::move(other.isOut)){}
BaseNode &BaseNode::operator=(const BaseNode &other){
    ip = other.ip; port = other.port; fd = other.fd;
    recvBuff = other.recvBuff; sendBuff = other.sendBuff;
    return *this;
}
BaseNode::~BaseNode(){
    if(fd != 0){
        close(fd);
        fd = 0;
    }
}
void BaseNode::Init(){
    isOut = false;
    status = MSGSTATUS::HANDLE_INIT;
}
int BaseNode::ReadProcess(){
    while(true){
        ssize_t recvSize = recvBuff.ReadFd(fd, &saveErrno);
        if(recvSize == 0)
            return 2;
        if(recvSize == -1){
            if(saveErrno != EAGAIN){
                return 2;
            }
            return 1;
        }

        if(status == HANDLE_INIT)
            ParseHead_();
        if(status == HANDLE_BODY)
            ParseBody_();
        if(status == HANDLE_COMPLATE){
            if(isOut){
                Init();
                return 0;
            }
            else{
                Init();
                return 1;
            }
        }
        if(status == HANDLE_ERROR)
            return 2;
    }
    return 0;
}
int BaseNode::WriteProcess(){
    ssize_t sendSize = 0;
    sendSize = sendBuff.WriteFd(static_cast<int>(fd), &saveErrno);
    if(sendSize == 0)
        return 2;
    if(sendSize < 0){
        if(saveErrno == EAGAIN)
            return 1;
        return 2;
    }
    if(sendBuff.UnHandleBytes())
        return 1;
    else return 0;
}


bool BaseNode::InitListenSocket(uint16_t &sockt, int port){
    int ret;
    struct sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    listenAddr.sin_port = htons(port);

    sockt = socket(AF_INET, SOCK_STREAM, 0);
    if(sockt == -1){
        LOG_ERROR("[socket] init ERROR");
        return false;
    }

    int optval = 1;
    ret = setsockopt(sockt, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1){
        LOG_ERROR("[socket] set ERROR");
        return false;
    }

    ret = bind(sockt, (sockaddr*)&listenAddr, sizeof(listenAddr));
    if(ret == -1){
        LOG_ERROR("[socket] bind ERROR");
        return false;
    }

    ret = listen(sockt, 4096);
    if(ret == -1){
        LOG_ERROR("[socket] listen ERROR");
        return false;
    }

    setNonBlocking(sockt);

    return true;
}