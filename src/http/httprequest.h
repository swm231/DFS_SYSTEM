#pragma once

#include <sys/stat.h>
#include <errno.h>

#include "../single/message.h"

class HttpRequest : public Request{
public:
    HttpRequest() : Request() {}
    ~HttpRequest() = default;

    void Init();

    bool IsKeepAlice() const;

    bool parse();

    void Append(const char *str, size_t len);


private:
    int code_;
    bool isKeepAlive_;

    std::string path_;

};