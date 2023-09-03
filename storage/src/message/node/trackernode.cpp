#include "../node.h"
#include "../config.h"
#include "../storagepack.h"
#include "../syntracker.h"

TrackerNode::TrackerNode(uint32_t _ip, uint16_t _port, int16_t _fd):BaseNode(_ip, _port, _fd){}
TrackerNode::TrackerNode(TrackerNode&& other): BaseNode(std::move(other)){}
TrackerNode &TrackerNode::operator=(const TrackerNode &other){
    ip = other.ip; port = other.port; fd = other.fd;
    return *this;
}
TrackerNode::~TrackerNode(){
    int pos = -1;
    for(int i = 0; i < Conf::Instance().tracker.size(); i++)
        if(fd == Conf::Instance().tracker[i]->fd){
            pos = i;
            break;
        }
    if(pos != -1)
        Conf::Instance().tracker.erase(Conf::Instance().tracker.begin() + pos);
}
void TrackerNode::ReadyNEW(){
    sendBuff.Append("NEW");
    sendBuff.Append(" ");
    sendBuff.Append(Conf::Instance().group_name);
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Conf::Instance().task_port));
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Conf::Instance().http_port));
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Conf::Instance().data_capacity));
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Conf::Instance().data_used));
    sendBuff.Append("\r\n");
}
void TrackerNode::ReadyKEP(){
    sendBuff.Append("KEP");
    sendBuff.Append(" ");
    sendBuff.Append("\r\n");
}
void TrackerNode::ReadySynSelf(){
    sendBuff.Append("CHG");
    sendBuff.Append(" ");
    sendBuff.Append("self");
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(STORAGE_STATUS_ACTIVE));
    sendBuff.Append("\r\n");
}
void TrackerNode::ReadySynUser(const std::string &username){
    sendBuff.Append("CHG");
    sendBuff.Append(" ");
    sendBuff.Append("user");
    sendBuff.Append(" ");
    sendBuff.Append(username);
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(STORAGE_STATUS_ACTIVE));
    sendBuff.Append("\r\n");
}
void TrackerNode::ParseHead_(){
    Init();
    std::string str = std::string(recvBuff.Peek(), 3);
    recvBuff.AddHandled(4);
    if(str == "new")
        DealTranew_();
    else if(str == "kep")
        DealTrakep_();
    status = MSGSTATUS::HANDLE_COMPLATE;
}
void TrackerNode::DealTranew_(){
    std::string byteStreamString = recvBuff.UnhandleToStr();
    std::istringstream stream(byteStreamString);

    int num;
    stream >> num;
    for(int i = 0; i < num; i++){
        uint32_t _ip; uint16_t _port, _fd;
        stream >> _ip >> _port;
        _fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(_ip);
        addr.sin_port = htons(_port);
        if(connect(_fd, (sockaddr*)&addr, sizeof addr) < 0){
            LOG_ERROR("[socket] tracker socket connect error");
            continue;
        }
        StorageNode *ptr = new StorageNode(_ip, _port, _fd);
        StorageNode::group[_fd] = ptr;
        Epoll::Instance().addFd(_fd, ptr, Epoll::connEvent_);

        StorageNode::group[_fd]->ReadyStoNEW();
        Epoll::Instance().modFd(_fd, ptr, Epoll::connEvent_ | EPOLLOUT);
        LOG_INFO("[group] 同组信息:%s %ld", inet_ntoa(addr.sin_addr), _port);
    }
    // TODO 自动更新
    // if(num == 0){
    //     isOut = true;
    //     SyncTracker::SyncSelf();
    // }
    isOut = true;
    SyncTracker::SyncSelf();
}
void TrackerNode::DealTrakep_(){ }
