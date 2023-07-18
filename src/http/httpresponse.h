#pragma once

#include <string>
#include <fstream>

#include "../single/message.h"
#include "../single/epoll.h"

class HttpResponse : public Response {
public:
    HttpResponse();
    ~HttpResponse();
    void Init();
    void MaskeResponse();

private:
    int code_;
    bool isKeepAlive_;

    std::string path_;

};