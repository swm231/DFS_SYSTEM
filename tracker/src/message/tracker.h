#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <list>

#include "../single/encipher.h"
#include "../pool/sqlconnraii.h"
#include "../buffer/buffer.h"
#include "../log/log.h"
#include "message.h"
#include "storagepack.h"

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
    PUBLIC_SERVER,
    PRIVATE_SREVER,
    PATH_OTHER
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
    _PWDERR, _PUBLIC, _PRIVATE, _REGISTER, _TITLE, _WELCOME, _LISTEND,
    _HTMLEND, _JSEND, _PUBLICJS, _PRIVATEJS
};


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


class FdNode{
public:
    FdNode();
    FdNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    FdNode(FdNode &&other);
    virtual ~FdNode();

    virtual void Init();
    virtual void Close() = 0;

    virtual int ReadProcess();
    virtual int WriteProcess();

    uint32_t ip;
    uint16_t port, fd;
    int saveErrno;
    Buffer recvBuff, sendBuff;

    MSGSTATUS status;
    bool isOut;

    static bool InitListenSocket(uint16_t &sockt, int port);

private:
    virtual void ParseHead_() {  }
    virtual void ParseBody_() {  }
};

class StorageNode: public FdNode{
public:
    StorageNode(){}
    StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    StorageNode(StorageNode &&other);
    ~StorageNode();

    void Init(){

    }
    void Close(){

    }

    void Update(){

    }

    uint16_t TaskPort, HttpPort;
    uint32_t TotalSize, UsedSize;
    int Status;
    std::string groupName;
private:
    void SignalAll_(int signum);

    void ParseHead_();

    void DealNEW_(const char*);
    void DealKEP_(const char*);
};


class ListenHttp: public FdNode{
public:
    ListenHttp(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenHttp(){  }

    void Close(){  }
private:
    int ReadProcess();
    int WriteProcess();
};

class ListenTask: public FdNode{
public:
    ListenTask(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenTask(){  }

    void Close(){  }
private:
    int ReadProcess();
    int WriteProcess();
};


class Message{
public:
    static char CR[];
    static char CRLF[];
    static std::unordered_map<int, StorageNode*> storageNode;
    static std::unordered_map<std::string, std::list<int> > group;
};