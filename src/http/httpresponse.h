#pragma once

#include <dirent.h>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/sendfile.h>

#include "../single/message.h"
#include "../single/epoll.h"

class HttpResponse : public Response {
public:
    HttpResponse();
    ~HttpResponse();
    void Init(std::string path_, std::string rescouce, bool isKeepAlice, int code);

    int process();

private:
    void Parse_();

    void FindHtml_();
    void AddStateLine_();
    void AddHeader_();
    void AddContent_();

    std::string GetFileType_();
    void GetFileListPage_();
    void GetFileVec_(const std::string &path, std::vector<std::string> &fileList);

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string resource_;
    std::string resPath_;

    struct stat fileSata_;
    int fileMsgFd;

};