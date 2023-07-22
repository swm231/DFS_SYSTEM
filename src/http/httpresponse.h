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
    void Init(const std::string &path, const std::string &rescouce, bool isKeepAlice, int code);

    int process();

    bool IsKeepAlice() const{
        return isKeepAlive_;
    }

private:
    void Parse_();

    void FindHtml_();
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

    struct stat fileSata_;
    int fileMsgFd;

};