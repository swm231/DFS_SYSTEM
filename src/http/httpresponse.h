#pragma once

#include <string>
#include <fstream>
#include <sys/stat.h>

#include "../single/message.h"
#include "../single/epoll.h"

class HttpResponse : public Response {
public:
    HttpResponse();
    ~HttpResponse();
    void Init(std::string path_, std::string rescouce, bool isKeepAlice, int code);
    void MaskeResponse();

private:
    void FindHtml_();
    void AddStateLine_();
    void AddHeader_();
    void AddContent_();

    std::string GetFileType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string resource_;

    struct stat fileSata_;
    

};