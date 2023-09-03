#pragma once

#include <unordered_map>

#include "../message/enumstatus.h"
#include "../message/storagemess.h"
#include "../message/consistenthash.h"
#include "../single/heaptimer.h"
#include "../single/encipher.h"
#include "../pool/sqlconnraii.h"

class Request{
public:
    Request(){}

public:
    MSGSTATUS HeadStatus;
    MSGBODYTYPE BodySatus;

    static const std::unordered_map<std::string, BEHAVIOR> DEFAULT_BEHAVIOR;
    static const std::unordered_map<std::string, PATH> DEFAULT_PATH;
};

class Response{
public:
    Response(){}

public:
    MSGSTATUS HeadStatus;
    MSGBODYTYPE BodySatus;

    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<HTML_ENUM, const char*> HTML_RESOURCE;
};


class StorageNode;
class HttpMessage{
public:
    int fd_;
    int SaveErrno;

    HttpMessage(){}
    void Init(){
        Method = METHOD::METHOD_OTHER, Path = PATH::PATH_OTHER, 
        Behavior = BEHAVIOR::BEHAVIOR_OTHER, Version = VERSION::VERSION_OTHER;
        isSetCookie = false;
        UserName = PassWord = FileName = Groupname = "";
        MsgHeader.clear();
        code = 200;
    }

    void updateServer(){
        int delPos = -1;
        for(auto it: userServer[UserName]){
            if(delPos != -1){
                userServer[UserName].erase(delPos);
                delPos = -1;            
            }
            if(Message::storageNode.count(it.first) == 0)
                delPos = it.first;
        }
        std::string group_ = ConsistentHash::Instance().GetGroup(UserName);
        for(auto it : Message::group[group_])
            if(userServer[UserName].count(it) == 0)
                userServer[UserName][it] = Message::storageNode[it]->Status;
    }

    METHOD Method;
    PATH Path;
    BEHAVIOR Behavior;
    VERSION Version;

    // true表示用用户名密码登录，否表示用cookie登录
    bool isSetCookie; 

    int code;
    std::string UserName, PassWord;
    std::string FileName;
    std::string Cookie;
    std::string Groupname;

    std::unordered_map<std::string, std::string> MsgHeader;

    static std::unordered_set<std::string> onlieUser;
    // 用户名为first 键值
    static std::unordered_map<std::string, std::unordered_map<int, int> > userServer;
};
