#include "storageserver.h"


std::atomic<int> ThreadPool::free_;
char HttpMessage::CR[] = "\r";
char HttpMessage::CRLF[] = "\r\n";
std::unordered_map<int, StorageNode*> StorageNode::group;

FdNode::FdNode():ip(0), port(0), fd(0), saveErrno(0), status(MSGSTATUS::HANDLE_INIT), isOut(false){}
FdNode::FdNode(uint32_t _ip, uint16_t _port, uint16_t _fd): ip(_ip), port(_port), fd(_fd), 
    saveErrno(0), status(MSGSTATUS::HANDLE_INIT), isOut(false){}
FdNode::FdNode(FdNode &&other):ip(std::move(other.ip)), port(std::move(other.port)), fd(std::move(other.fd)),
    saveErrno(std::move(other.saveErrno)), recvBuff(std::move(other.recvBuff)), sendBuff(std::move(other.sendBuff)),
    status(std::move(other.status)), isOut(std::move(other.isOut)){}
FdNode &FdNode::operator=(const FdNode &other){
    ip = other.ip; port = other.port; fd = other.fd;
    recvBuff = other.recvBuff; sendBuff = other.sendBuff;
    return *this;
}
FdNode::~FdNode(){
    if(fd != 0){
        close(fd);
        fd = 0;
    }
}
void FdNode::Init(){
    isOut = false;
    status = MSGSTATUS::HANDLE_INIT;
}
int FdNode::ReadProcess(){
    while(true){
        ssize_t recvSize = recvBuff.ReadFd(fd, &saveErrno);
        if(recvSize == 0)
            return 2;
        if(recvSize == -1){
            if(saveErrno != EAGAIN)
                return 2;
            return 1;
        }

        if(status == HANDLE_INIT)
            ParseHead_();
        if(status == HANDLE_BODY)
            ParseBody_();
        if(status == HANDLE_COMPLATE){
            Init();
            if(isOut)
                return 0;
            else
                return 1;
        }
        if(status == HANDLE_ERROR)
            return 2;
    }
    return 0;
}
int FdNode::WriteProcess(){
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

TaskNode::TaskNode(uint32_t _ip, uint16_t _port, uint16_t _fd): FdNode(_ip, _port, _fd){}
TaskNode::~TaskNode(){}
void TaskNode::Init(){
    status = MSGSTATUS::HANDLE_INIT;
    hasSaveLen_ = BodyLen_ = 0;
}
void TaskNode::Close(){
    if(ofs.is_open() == false)
        ofs.close();
}
void TaskNode::ParseHead_(){
    const char *lineEnd = std::search(recvBuff.Peek(), recvBuff.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
    if(lineEnd == recvBuff.BeginWriteConst())
        return;
    
    std::string str(recvBuff.Peek(), 3);
    recvBuff.AddHandled(4);
    if(str == "NEW")
        DealNEW_(lineEnd);
    else if(str == "DEL")
        DealDEL_(lineEnd);
    else if(str == "UPL")
        DealUPL_(lineEnd);
}
void TaskNode::DealNEW_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
    std::istringstream stream(byteStreamString);

    uint16_t _port, _fd;
    stream >> _port;
    _fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(ip);
    addr.sin_port = htons(_port);
    if(connect(_fd, (sockaddr*)&addr, sizeof addr) < 0){
        LOG_ERROR("[socket] tracker socket connect error");
        return;
    }
    StorageNode *ptr = new StorageNode(ip, _port, _fd);
    StorageNode::group[_fd] = ptr;
    Epoll::Instance().addFd(_fd, ptr, Epoll::connEvent_);
    LOG_INFO("[storage] 新Storage ip:%s port:%ld", inet_ntoa(addr.sin_addr), _port);
    status = MSGSTATUS::HANDLE_COMPLATE;
}
void TaskNode::DealDEL_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
    std::istringstream stream(byteStreamString);

    std::string path, username, filename;
    stream >> path >> username >> filename;
    if(path == "pub")
        remove((Conf::Instance().data_path + "/public/" + filename).c_str());
    else if(path == "pri")
        remove((Conf::Instance().data_path + "/private/" + username + "/" + filename).c_str());

    status = MSGSTATUS::HANDLE_COMPLATE;
    LOG_INFO("[syn] 删除文件:%s %s %s", path.c_str(), username.c_str(), filename.c_str());
}
void TaskNode::DealUPL_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
    std::istringstream stream(byteStreamString);

    std::string path, username, filename;
    stream >> path >> username >> filename >> BodyLen_;
    std::string filePath;
    if(path == "pub")
        filePath = Conf::Instance().data_path + "/public/" + filename;
    else if(path == "pri")
        filePath = Conf::Instance().data_path + "/private/" + username + "/" + filename;
    ofs.open(filePath, std::ios::out | std::ios::app | std::ios::binary);
    status = MSGSTATUS::HANDLE_BODY;
    LOG_INFO("[syn] 上传文件:%s", filePath.c_str());
}
void TaskNode::ParseBody_(){
    long long saveLen = std::min((long long)recvBuff.UnHandleBytes(), BodyLen_ - hasSaveLen_);
    ofs.write(recvBuff.Peek(), saveLen);
    hasSaveLen_ += saveLen;
    if(hasSaveLen_ >= BodyLen_)
        status = MSGSTATUS::HANDLE_COMPLATE;
}


TrackerNode::TrackerNode(uint32_t _ip, uint16_t _port, int16_t _fd):FdNode(_ip, _port, _fd){}
TrackerNode::TrackerNode(TrackerNode&& other): FdNode(std::move(other)){}
TrackerNode &TrackerNode::operator=(const TrackerNode &other){
    ip = other.ip; port = other.port; fd = other.fd;
    return *this;
}
TrackerNode::~TrackerNode(){
    int pos = -1;
    for(int i = 0; i < Conf::Instance().tracker.size(); i++)
        if(fd == Conf::Instance().tracker[i].fd){
            pos = i;
            break;
        }
    if(pos != -1)
        Conf::Instance().tracker.erase(Conf::Instance().tracker.begin() + pos);
}
void TrackerNode::ReadyTraNEW(){
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
void TrackerNode::ReadyTraKEP(){
    sendBuff.Append("KEP");
    sendBuff.Append(" ");
    sendBuff.Append("\r\n");
}
void TrackerNode::ParseHead_(){
    std::string str = std::string(recvBuff.Peek(), 3);
    recvBuff.AddHandled(4);
    if(str == "new")
        DealTranew_();
    else if(str == "kep")
        DealTrakep_();
}
// group失效
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
}
void TrackerNode::DealTrakep_(){ }


StorageNode::StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd): 
    FdNode(_ip, _port, _fd), fileMsgfd_(-1), hasSentLen_(0){}
StorageNode::StorageNode(StorageNode &&other) : FdNode(std::move(other)), hasSentLen_(std::move(other.hasSentLen_)),
    fileMsgfd_(std::move(other.fileMsgfd_)){}
StorageNode::~StorageNode(){
    if(group.count(fd))
        group.erase(fd);
}
void StorageNode::Init(){
    status = MSGSTATUS::HANDLE_INIT;
    hasSentLen_ = BodyLen = 0;
    fileMsgfd_ = -1;
}
void StorageNode::Close(){
    if(fileMsgfd_ != -1){
        close(fileMsgfd_);
        fileMsgfd_ = -1;
    }
}
void StorageNode::ReadyStoNEW(){
    Init();
    sendBuff.Append("NEW");
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Conf::Instance().task_port));
    sendBuff.Append("\r\n");
    status = MSGSTATUS::HANDLE_HEAD;
}
void StorageNode::ReadySYN(BEHAVIOR behavior, PATH path, const std::string &username, const std::string &filename){
    Init();
    if(behavior == BEHAVIOR::DELETE){
        sendBuff.Append("DEL ");
        if(path == PATH::PUBLIC)
            sendBuff.Append("pub username ");
        else
            sendBuff.Append("pri "), sendBuff.Append(username), sendBuff.Append(" ");
        sendBuff.Append(filename);
        sendBuff.Append("\r\n");
        status = MSGSTATUS::HANDLE_HEAD;
        return;
    }

    struct stat fileStat;
    sendBuff.Append("UPL ");
    if(path == PATH::PUBLIC){
        sendBuff.Append("pub username ");
        fileMsgfd_ = open((Conf::Instance().data_path + "/public/" + filename).c_str(), O_RDONLY);
    }
    else{
        sendBuff.Append("pri "), sendBuff.Append(username), sendBuff.Append(" ");
        fileMsgfd_ = open((Conf::Instance().data_path + "/private/" + username + "/" + filename).c_str(), O_RDONLY);
    }
    sendBuff.Append(filename);
    sendBuff.Append(" ");
    fstat(fileMsgfd_, &fileStat);
    sendBuff.Append(std::to_string(fileStat.st_size));
    sendBuff.Append("\r\n");
    hasSentLen_ = 0; BodyLen = fileStat.st_size;
    status = MSGSTATUS::HANDLE_HEAD;
}
int StorageNode::WriteProcess(){
    if(status == MSGSTATUS::HANDLE_HEAD){
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
        else if(fileMsgfd_ == -1)
            status = MSGSTATUS::HANDLE_COMPLATE;
        else
            status = MSGSTATUS::HANDLE_BODY;
    }
    if(status == MSGSTATUS::HANDLE_BODY){
        while(true){
            long long sentLen = hasSentLen_;
            sentLen = sendfile(fd, fileMsgfd_, (off_t*)&sentLen, BodyLen - sentLen);
            if(sentLen == -1){
                if(errno != EAGAIN)
                    return 2;
                return 1;
            }
            hasSentLen_ += sentLen;
            if(hasSentLen_ >= BodyLen){
                status = MSGSTATUS::HANDLE_COMPLATE;
                break;;
            }
        }
    }
    if(status == MSGSTATUS::HANDLE_COMPLATE){
        Close();
        return 0;
    }
    else
        return 2;
}

int ListenHttp::ReadProcess(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd_ = accept(fd, (sockaddr*)&addr, &len);
    if(fd_ <= 0) return 1;
    HttpConn *ptr = new HttpConn();
    ptr->Init(fd_, addr);
    setNonBlocking(fd_);
    Epoll::Instance().addFd(fd_, ptr, Epoll::connEvent_ | EPOLLIN);
    return 1;
}
int ListenHttp::WriteProcess(){ return 0; }


int ListenTask::ReadProcess(){
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    int fd_ = accept(fd, (sockaddr*)&addr, &len);
    if(fd_ <= 0) return 1;
    TaskNode *ptr = new TaskNode(ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port), fd_);
    setNonBlocking(fd_);
    Epoll::Instance().addFd(fd_, ptr, Epoll::connEvent_ | EPOLLIN);
    return 1;
}
int ListenTask::WriteProcess(){ return 0; }


bool FdNode::InitListenSocket(uint16_t &sockt, int port){
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
