#pragma once

#include <dirent.h>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "httpmessage.h"
#include "../message/enumstatus.h"
#include "../single/epoll.h"

class HttpResponse : public Response {
public:
    HttpResponse(HttpMessage *);
    ~HttpResponse();

    void Init();
    void Close();
    int process();

private:
    void Parse_();

    void AddStateLine_();
    void AddHeader_();
    void AddContent_();

    std::string GetTracker_();

    struct stat fileSata_;
    int fileMsgFd;

    Buffer beforeBodyMsg;
    int beforeBodyMsgLen;

    Buffer BodyMsg;
    unsigned long long BodyMsgLen;

    unsigned long long HasSentLen;

    HttpMessage *Message_;
};