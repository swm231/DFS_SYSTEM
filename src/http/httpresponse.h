#pragma once

#include <dirent.h>
#include <string>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "message.h"
#include "../single/epoll.h"
#include "../resources/html.h"

class HttpResponse : public Response {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string &srcDir, const std::string &resDir, const std::string action, const std::string &resource,
            const std::string &username, int isSetCookie, const std::string &cookie, bool isKeepAlice, int code);
    void Close();
    int process();

    bool IsKeepAlice() const{
        return isKeepAlive_;
    }

private:
    void Parse_();

    void AddStateLine_();
    void AddHeader_();
    void AddContent_();
    void AddFileStream_(const std::string &fileName);

    std::string GetFileType_();
    void GetFileListPage_();
    void GetFileVec_(const std::string &path, std::vector<std::string> &fileList);
    void GetHtmlPage_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string resource_;
    std::string resPath_;
    std::string action_;

    struct stat fileSata_;
    int fileMsgFd;

    std::string username_;
    std::string cookie_;
    int isSetCookie_;
};