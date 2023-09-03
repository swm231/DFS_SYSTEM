#pragma once

#include <unordered_map>
#include <fstream>

#include "../message/basenode.h"
#include "../message/node.h"
#include "../message/config.h"
#include "../single/encipher.h"

class Request{
public:
    Request(){}

public:
    MSGSTATUS HeadStatus;
    FILEMSGESTATUS FileStatus;

    static const std::unordered_map<std::string, BEHAVIOR> DEFAULT_BEHAVIOR;
};

class Response{
public:
    Response(){}

public:
    MSGSTATUS HeadStatus;

    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<HTML_ENUM, const char*> HTML_RESOURCE;
};

class HttpMessage{
public:
    int fd_;
    int SaveErrno;

    HttpMessage(){}
    void Init(){
        Method = METHOD::METHOD_OTHER, Path = PATH::PATH_OTHER, 
        Behavior = BEHAVIOR::BEHAVIOR_OTHER, Version = VERSION::VERSION_OTHER;
        isPrivateUser = false;
        UserName = PassWord = FileName = "";
        MsgHeader.clear();
        code = 200;
    }

    METHOD Method;
    PATH Path;
    BEHAVIOR Behavior;
    VERSION Version;

    bool isPrivateUser;

    int code;
    std::string UserName, PassWord;
    std::string FileName;
    std::string Cookie;

    std::unordered_map<std::string, std::string> MsgHeader;
    
    static char CR[];
    static char CRLF[];
};
