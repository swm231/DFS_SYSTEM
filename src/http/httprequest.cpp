#include "httprequest.h"

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg.append(str, len);
}