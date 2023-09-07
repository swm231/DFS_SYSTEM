#include "../node.h"
#include "../config.h"
#include "../syntracker.h"
#include "../../http/httpmessage.h"

TaskNode::TaskNode(uint32_t _ip, uint16_t _port, uint16_t _fd): BaseNode(_ip, _port, _fd),
    hasSaveLen_(0), BodyLen_(0), fp(nullptr){}
TaskNode::~TaskNode(){}
void TaskNode::Init(){
    isOut = false;
    status = MSGSTATUS::HANDLE_INIT;
    fileStatus = FILEMSGESTATUS::FILE_BEGIN;
    hasSaveLen_ = BodyLen_ = 0;
    username_ = "";
}
void TaskNode::Close(){
    if(fp){
        fclose(fp);
        fp = nullptr;
    }
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
void TaskNode::ParseBody_(){
    const char *lineEnd;
    std::string strLine;

    if(fileStatus == FILEMSGESTATUS::FILE_BEGIN){
        lineEnd = std::search(recvBuff.Peek(), recvBuff.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
        if(lineEnd == recvBuff.BeginWriteConst())
            return;
        strLine = std::string(recvBuff.Peek(), lineEnd - recvBuff.Peek());
        if(strLine == boundary + "--"){
            fileStatus = FILEMSGESTATUS::FILE_HEAD;
            recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
        }
        else{
            Init();
            recvBuff.AddHandledAll();
            return;
        }
    }

    if(fileStatus == FILEMSGESTATUS::FILE_HEAD){
        while(true){
            int saveLen = recvBuff.UnHandleBytes();
            if(saveLen == 0)
                break;

            lineEnd = std::search(recvBuff.Peek(), recvBuff.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
            if(lineEnd != recvBuff.BeginWriteConst()){
                int endBoundaryLen = boundary.size() + 6;
                if(recvBuff.BeginWriteConst() - lineEnd >= endBoundaryLen){
                    if(std::string(lineEnd, endBoundaryLen) == "\r\n--" + boundary + "\r\n"){
                        if(lineEnd == recvBuff.Peek()){
                            fileStatus = FILE_COMPLATE;
                            break;
                        }
                        saveLen = lineEnd - recvBuff.Peek();
                    }
                    else if(lineEnd + 2 < recvBuff.BeginWriteConst()){
                        lineEnd = std::search(lineEnd + 2, recvBuff.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
                        if(lineEnd != recvBuff.BeginWriteConst())
                            saveLen = lineEnd - recvBuff.Peek();
                    }
                }
                else{
                    if(lineEnd == recvBuff.Peek())
                        break;
                    saveLen = lineEnd - recvBuff.Peek();
                }
            }
            size_t err = fwrite(recvBuff.Peek(), sizeof(char), saveLen, fp);
            recvBuff.AddHandled(saveLen);
        }
    }

    if(fileStatus == FILEMSGESTATUS::FILE_COMPLATE){
        // 读取文件尾标志
        recvBuff.AddHandled(boundary.size() + 6);
        status = MSGSTATUS::HANDLE_COMPLATE;
        // 发送同步完成
        SyncTracker::SyncUser(username_);
        // recvBuff.AddHandledAll();
        // 更新日志
        RingLog::Instance().FreeLog(ringlogAddr_Rollback);
        rollbacklog.ClearOrder();
        Close();
        LOG_INFO("[task] 文件处理完成");
        return;
    }
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
    status = MSGSTATUS::HANDLE_COMPLATE;

    LOG_INFO("[storage] 新Storage ip:%s port:%ld", inet_ntoa(addr.sin_addr), _port);
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
    SyncTracker::SyncUser(username);

    LOG_INFO("[syn] 删除文件:%s %s %s", path.c_str(), username.c_str(), filename.c_str());
}
void TaskNode::DealUPL_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
    std::istringstream stream(byteStreamString);

    std::string path, username, filename;
    stream >> path >> username >> filename >> BodyLen_ >> boundary;
    username_ = username;
    std::string filePath;
    // 设置路径，同时设置日志
    if(path == "pub")
        filePath = Conf::Instance().data_path + "/public/" + filename,
        rollbacklog.MakeOrder(true, "", filename);
    else if(path == "pri")
        filePath = Conf::Instance().data_path + "/private/" + username + "/" + filename,
        rollbacklog.MakeOrder(true, username, filename);
    ringlogAddr_Rollback = RingLog::Instance().WriteRollbackLog(rollbacklog);
    fp = fopen(filePath.c_str(), "wb");
    status = MSGSTATUS::HANDLE_BODY;

    LOG_INFO("[syn] 上传文件:%s", filePath.c_str());
}