#pragma once

#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>

#include "../single/message.h"

class HttpRequest : public Request{
public:
    HttpRequest() : Request() {}
    ~HttpRequest() = default;

    void Init(int fd);

    bool IsKeepAlice() const;

    int parse();

    void Append(const char *str, size_t len);


private:
    int fd_;

    int code_;
    bool isKeepAlive_;

    std::string path_;

};