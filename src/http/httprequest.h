#pragma once

#include <sys/stat.h>

#include "../single/message.h"

class HttpRequest : public Request{
public:
    HttpRequest() : Request() {}
    ~HttpRequest() = default;

    void Append(const char *str, size_t len);

private:

};