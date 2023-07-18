#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

// 表示http信息处理状态
enum MSGSTATUS{
    HANDLE_INIT,
    HANDLE_HEAD,
    HANDLE_BODY,
    HANDLE_COMPLATE,
    HANDLE_ERROE,
};

// 表示消息体类型
enum MSGBODYTYPE{
    FIILE_TYPE,
    HTML_TYPE,
    EMPTY_TYPE,
};

class Message{
public:
    Message() : Status(HANDLE_INIT){}

public:
    MSGSTATUS Status;

    std::unordered_map<std::string, std::string> MsgHeader;
};

class Request : public Message{
public:
    Request() : Message(), ContentLength(0){}

    void setRequestLine(const std::string &Line){
        std::istringstream lineStream(Line);
        lineStream >> Method;
        lineStream >> Resource;
        lineStream >> Version;

        if(Resource == "/")
            Resource = "/index.html";
        else{
            if(DEFAULT_HTML.count(Resource))
                Resource += ".html";
        }
    }

    void addHeaderOpt(const std::string &Line){
        std::istringstream lineStream(Line);

        std::string key, value;

        lineStream >> key;
        key.pop_back();

        lineStream.get();

        getline(lineStream, value);
        value.pop_back();

        if(key == "Content-Length")
            ContentLength = std::stoll(value);
        
        MsgHeader[key] = value;
    }

public:
    // buffer
    std::string RecvMsg;

    std::string Method;
    std::string Resource;
    std::string Version;

    long long ContentLength;
    long long MsgBodyRecvLen;

    static const std::unordered_set<std::string> DEFAULT_HTML;
};

class Response : public Message{
public:
    Response() : Message(), HasSendLen(0){}

public:
    // 状态行
    std::string Version = "HTTP/1.1";
    std::string StatusCode;
    std::string StatusDes;

    MSGBODYTYPE bodyType;
    std::string bodyFileName;

    std::string beforeBodyMsg;
    int beforeBodyMsgLen;

    std::string MsgBody;
    unsigned long long MsgBodyLen;

    int FileMsgFd;

    unsigned long long HasSendLen;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
};