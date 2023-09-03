#include <sstream>

#include "../node.h"
#include "../storagepack.h"
#include "../storagemess.h"
#include "../consistenthash.h"
#include "../../http/httpmessage.h"

StorageNode::StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd): BaseNode(_ip, _port, _fd), Status(STORAGE_STATUS_INIT){
    Status = STORAGE_STATUS_WAIT_SYNC;
}
StorageNode::~StorageNode(){
    std::list<int>::iterator pos;
    bool flag = false;
    for(std::list<int>::iterator it = Message::group[groupName].begin(); 
        it != Message::group[groupName].end(); it++)
            if(*it == fd){
                pos = it;
                flag = true;
                break;
            }
    if(flag)
        Message::group[groupName].erase(pos);
    if(Message::storageNode.count(fd))
        Message::storageNode.erase(fd);
}
void StorageNode::ParseHead_(){
    const char *lineEnd = std::search(recvBuff.Peek(), recvBuff.BeginWriteConst(), Message::CRLF, Message::CRLF + 2);
    if(lineEnd == recvBuff.BeginWriteConst())
        return;

    std::string str(recvBuff.Peek(), 3);
    recvBuff.AddHandled(4);
    if(str == "NEW")
        DealNEW_(lineEnd);
    else if(str == "KEP")
        DealKEP_(lineEnd);
    else if(str == "CHG")
        DealCHG_(lineEnd);
}
void StorageNode::DealNEW_(const char *lineEnd){
    Status = STORAGE_STATUS_WAIT_SYNC;
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);

    std::istringstream stream(byteStreamString);
    stream >> groupName >> TaskPort >> HttpPort >> TotalSize >> UsedSize;

    // 准备当前分组的服务器信息
    sendBuff.Append("new");
    sendBuff.Append(" ");
    sendBuff.Append(std::to_string(Message::group[groupName].size()));
    sendBuff.Append(" ");
    for(auto it:Message::group[groupName]){
        sendBuff.Append(std::to_string(Message::storageNode[it]->ip));
        sendBuff.Append(" ");
        sendBuff.Append(std::to_string(Message::storageNode[it]->TaskPort));
        sendBuff.Append(" ");
    }
    sendBuff.Append("\r\n");

    if(Message::group[groupName].size() == 0 && groupName != "public")
        ConsistentHash::Instance().AddNode(groupName);

    Message::group[groupName].push_back(fd);
    isOut = true;
    status = HANDLE_COMPLATE;
}
void StorageNode::DealKEP_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);
    sendBuff.Append("kep");
    sendBuff.Append("\r\n");
    isOut = true;
    status = HANDLE_COMPLATE;
}
void StorageNode::DealCHG_(const char *lineEnd){
    std::string byteStreamString(recvBuff.Peek(), lineEnd - recvBuff.Peek());
    recvBuff.AddHandled(lineEnd - recvBuff.Peek() + 2);

    std::string method, username;
    int chg_status;
    std::istringstream stream(byteStreamString);
    stream >> method;
    if(method == "user"){
        stream >> username >> chg_status;
        HttpMessage::userServer[username][fd] = chg_status;
        if(HttpMessage::onlieUser.count(username) == 0){
            bool flag = false;
            for(auto it: HttpMessage::userServer[username])
                if(it.second != STORAGE_STATUS_ACTIVE)
                    flag = true;
            if(flag == false)
                HttpMessage::userServer.erase(username);
        }
    }
    else if(method == "self"){
        stream >> chg_status;
        {
            // 加写锁
            WriterLockRAII locker(Message::lock);
            Status = chg_status;
            Message::changeFLAG();
        }
        Message::threadGO();
    }
    status = HANDLE_COMPLATE;
}
void StorageNode::SignalAll_(int signum){
    for(auto it : Message::group[groupName])
        Message::storageNode[it]->Status = signum;
}