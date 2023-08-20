#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "../single/encipher.h"
#include "../buffer/buffer.h"
#include "../log/log.h"

// 表示http信息处理状态
enum MSGSTATUS{
    HANDLE_INIT,
    HANDLE_HEAD,
    HANDLE_BODY,
    HANDLE_COMPLATE,
    HANDLE_ERROR,
};

// 表示消息体类型
enum MSGBODYTYPE{
    FILE_TYPE,
    TEXT_TYPE,
    EMPTY_TYPE,
};

// 表示文件处理过程
enum FILEMSGESTATUS{
    FILE_BEGIN,
    FILE_HEAD,
    FILE_CONTENT,
    FILE_COMPLATE
};

enum METHOD{
    GET,
    POST,
    METHOD_OTHER,
};

enum PATH{
    ROOT,
    PUBLIC,
    PRIVATE,
    LOGIN,
    REGISTER,
    LOGOUT,
    WELCOME,
    NAMERR,
    PWDERR,
    PATH_OTHER,
};

enum BEHAVIOR{
    UPLOAD,
    DOWNLOAD,
    DELETE,
    BEHAVIOR_OTHER,
};

enum VERSION{
    _0_9,
    _1_0,
    _1_1,
    _2_0,
    VERSION_OTHER,
};

enum HTML_ENUM{
    _404_, _403_, _400_, _HEAD, _HEAD_, _INDEX, _LOGIN, _LOGOUT, _NAMERR,
     _PWDERR, _PUBLIC, _PRIVATE, _REGISTER, _TITLE, _WELCOME, _LISTEND 
};

class Message{
public:
    Message() : HeadStatus(HANDLE_INIT), BodySatus(EMPTY_TYPE){}

public:
    MSGSTATUS HeadStatus;
    MSGBODYTYPE BodySatus;

    static char CR[];
    static char CRLF[];
};


class Request : public Message{
public:
    Request() : Message(){}

public:
    FILEMSGESTATUS FileStatus;
    
    static const std::unordered_map<std::string, BEHAVIOR> DEFAULT_BEHAVIOR;
    static const std::unordered_map<std::string, PATH> DEFAULT_PATH;
};

class Response : public Message{
public:
    Response() : Message(){}

public:
    static int CookieOut;
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
        isSetCookie = false;
        UserName = PassWord = FileName = "";
        MsgHeader.clear();
        code = 200;
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

    std::unordered_map<std::string, std::string> MsgHeader;
};