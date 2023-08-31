#include "storage.h"

std::string StorageConf::group_name, StorageConf::data_path;
uint16_t StorageConf::http_port, StorageConf::task_port;
uint32_t StorageConf::data_capacity, StorageConf::data_used, StorageConf::bind_addr;

TrackerNode::TrackerNode(uint32_t _ip, uint16_t _port, int16_t _fd):
    ip(_ip), port(_port), fd(_fd){}
TrackerNode::TrackerNode(TrackerNode&& other){
    ip = std::move(other.ip);
    port = std::move(other.port);
    fd = std::move(other.fd);
    err = 0;
}

void TrackerNode::Close(){

}

void TrackerNode::ReadyNew(){
    sendBuff.Append("NEW");
    sendBuff.Append(" ");
    sendBuff.Append(StorageConf::group_name);
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(StorageConf::task_port));
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(StorageConf::http_port));
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(StorageConf::data_capacity));
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(StorageConf::data_used));
    sendBuff.Append("\r\n");
}

void TrackerNode::DealWrite(){
    ThreadPool::Instance().AddTask(std::bind(&TrackerNode::OnWrite_, this));
}

void TrackerNode::OnWrite_(){
    int ret = WriteProcess_();
    if(ret == 0)
        Epoll::Instance().modFd(fd, this, Epoll::trackerEvent_);
    else if(ret == 1)
        Epoll::Instance().modFd(fd, this, Epoll::trackerEvent_ | EPOLLOUT);
    else if(ret == 2)
        Close();
}

int TrackerNode::WriteProcess_(){
    ssize_t sendSize = 0;
    // std::cout << sendBuff.Peek()  << std::endl;
    sendSize = sendBuff.WriteFd(static_cast<int>(fd), &err);
    if(sendSize == 0)
        return 2;
    if(sendSize < 0){
        if(err == EAGAIN)
            return 1;
        return 2;
    }
    if(sendBuff.UnHandleBytes())
        return 1;
    else return 0;
}