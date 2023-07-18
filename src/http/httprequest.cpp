#include "httprequest.h"

const std::unordered_set<std::string> Request::DEFAULT_HTML{
        "/index", "/register", "/login", "/welcome"};

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg.append(str, len);
}