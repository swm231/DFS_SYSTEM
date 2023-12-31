#pragma once

#include <dirent.h>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "../single/epoll.h"
#include "../resources/html.h"
#include "httpmessage.h"

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
    void AddFileStream_(HTML_ENUM);

    std::string GetFileType_();
    void GetFileListPage_();
    void GetPubFileVec_(std::vector<std::string> &fileList);
    void GetPriFileVec_(std::vector<std::string> &fileList);
    void GetHtmlPage_();

    uint16_t HeaderAddPubAddr_();
    uint16_t HeaderAddPriAddr_();
    void HeaderAddAddr_(StorageNode *ptr);

    struct stat fileSata_;
    int fileMsgFd;

    Buffer beforeBodyMsg;
    int beforeBodyMsgLen;

    Buffer BodyMsg;
    unsigned long long BodyMsgLen;

    unsigned long long HasSentLen;

    HttpMessage *Message_;
};