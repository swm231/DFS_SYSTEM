#include "httprequest.h"

const std::unordered_set<std::string> Request::DEFAULT_HTML{
        "/index", "/register", "/login", "/welcome"};

void HttpRequest::Init(){
    Method = Resource = Version = "";
    Status = HANDLE_INIT;
    MsgHeader.clear();
    ContentLength = MsgBodyRecvLen = 0;
}

bool HttpRequest::parse(){
    std::cout << "开始解析" << std::endl;
    std::cout << RecvMsg << std::endl;

    std::string::size_type endIndex;

    // 请求行
    endIndex = RecvMsg.find("\r\n");
    setRequestLine(RecvMsg.substr(0, endIndex + 2));
    RecvMsg.erase(0, endIndex + 2);
    
    // 首部
    while(1){
        endIndex = RecvMsg.find("\r\n");
        std::string curLine = RecvMsg.substr(0, endIndex + 2);
        RecvMsg.erase(0, endIndex + 2);
        if(curLine == "\r\n")
            break;
    }
    std::cout << "解析完成，请求资源:" << Resource << std::endl;

    return true;
}

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg.append(str, len);
}

bool HttpRequest::IsKeepAlice() const{
    if(MsgHeader.count("Connection") == 1)
        return MsgHeader.find("Connection")->second == "keep-alive" && Version == "1.1";
    return false;
}