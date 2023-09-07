#include "../node.h"
#include "../config.h"
#include "../../consistlog/synlog/synlog.h"
#include "../../consistlog/ringlog/ringlog.h"

StorageNode::StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd): 
    BaseNode(_ip, _port, _fd), fileMsgfd_(-1), hasSentLen_(0){ Init(); }
StorageNode::StorageNode(StorageNode &&other) : BaseNode(std::move(other)), hasSentLen_(std::move(other.hasSentLen_)),
    fileMsgfd_(std::move(other.fileMsgfd_)){ Init(); }
StorageNode::~StorageNode(){
    if(group.count(fd))
        group.erase(fd);
}
void StorageNode::Init(){
    status = MSGSTATUS::HANDLE_INIT;
    fileStatus = FILEMSGESTATUS::FILE_BEGIN;
    hasSentLen_ = BodyLen = 0;
    fileMsgfd_ = -1;
    boundary = "";
}
void StorageNode::Close(){
    if(fileMsgfd_ != -1){
        close(fileMsgfd_);
        fileMsgfd_ = -1;
    }
}
int StorageNode::WriteProcess(){
    if(status == MSGSTATUS::HANDLE_INIT){
        // 从任务队列中取任务
        storTaskPack pack;
        {
            std::unique_lock<std::mutex> locker(mtx_);
            pack = taskQue_.front();
            taskQue_.pop();
        }
        if(pack.type == STORTASKTYPE::NEW)
            ReadyNEW_(pack);
        else if(pack.type == STORTASKTYPE::SYN)
            ReadySyn_(pack);
    }
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
        if(fileStatus == FILEMSGESTATUS::FILE_HEAD)
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
                    sendBuff.Append("\r\n--" + boundary + "\r\n");
                    fileStatus = FILEMSGESTATUS::FILE_CONTENT;
                    break;
                }
            }
        if(fileStatus == FILEMSGESTATUS::FILE_CONTENT){
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
            else 
                fileStatus = FILEMSGESTATUS::FILE_COMPLATE,
                status = MSGSTATUS::HANDLE_COMPLATE;
        }
    }
    if(status == MSGSTATUS::HANDLE_COMPLATE){
        finshTask();
        Close();
        Init();
        return 0;
    }
    else
        return 2;
}
void StorageNode::AddTask(const storTaskPack &task){
    std::unique_lock<std::mutex> locker(mtx_);
    taskQue_.push(task);
}
void StorageNode::AddTask(uint16_t port){
    std::unique_lock<std::mutex> locker(mtx_);
    taskQue_.emplace(storTaskPack(STORTASKTYPE::NEW, port));
}
void StorageNode::AddTask(const synPack &pack){
    std::unique_lock<std::mutex> locker(mtx_);
    taskQue_.emplace(storTaskPack(STORTASKTYPE::NEW, pack));
}
void StorageNode::ReadyNEW_(const storTaskPack &task){
    sendBuff.Append("NEW");
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Conf::Instance().task_port));
    sendBuff.Append("\r\n");
    status = MSGSTATUS::HANDLE_HEAD;

    finshTask = [] {};
}
void StorageNode::ReadySyn_(const storTaskPack &task){
    if(task.synpack.isUpload == false){
        sendBuff.Append("DEL ");
        if(task.synpack.username == "")
            sendBuff.Append("pub username ");
        else
            sendBuff.Append("pri "), sendBuff.Append(task.synpack.username), sendBuff.Append(" ");
        sendBuff.Append(task.synpack.filename);
        sendBuff.Append("\r\n");
    }
    else{
        struct stat fileStat;
        sendBuff.Append("UPL ");
        if(task.synpack.username == ""){
            sendBuff.Append("pub username ");
            fileMsgfd_ = open((Conf::Instance().data_path + "/public/" + task.synpack.filename).c_str(), O_RDONLY);
        }
        else{
            sendBuff.Append("pri "), sendBuff.Append(task.synpack.username), sendBuff.Append(" ");
            fileMsgfd_ = open((Conf::Instance().data_path + "/private/" + task.synpack.username + "/" + task.synpack.filename).c_str(), O_RDONLY);
        }
        boundary = encipher::getMD5(std::to_string(rand()), 16);
        
        sendBuff.Append(task.synpack.filename);
        sendBuff.Append(" ");
        fstat(fileMsgfd_, &fileStat);
        sendBuff.Append(std::to_string(fileStat.st_size));
        sendBuff.Append(" ");
        sendBuff.Append(boundary);
        sendBuff.Append("\r\n");
        sendBuff.Append(boundary + "--\r\n");
        BodyLen = fileStat.st_size;
    }
    status = MSGSTATUS::HANDLE_HEAD;
    fileStatus = FILEMSGESTATUS::FILE_HEAD;
    finshTask = std::bind(&StorageNode::FinshSyn_, this, task.synpack.ringLogId, task.synpack.synLogId);

    struct in_addr addr;
    addr.s_addr = htonl(ip);
    LOG_INFO("[storage] 准备同步 ip:%s port:%ld filename:%s", inet_ntoa(addr), port, task.synpack.filename.c_str());
}
void StorageNode::FinshSyn_(uint64_t ringLogId, uint64_t synLogId){
    SynLog::Instance().FinshLog(synLogId);

    RingLog::Instance().FreeLog(ringLogId);
}