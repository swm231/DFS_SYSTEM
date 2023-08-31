#pragma once

#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>
#include <cstring>
#include <regex>

#include "../message/tracker.h"
#include "../pool/sqlconnraii.h"

class HttpRequest : public Request{
public:
    HttpRequest(HttpMessage *Message) : Request() {
        Message_ = Message;
    }
    ~HttpRequest() = default;

    void Init();
    void Close();

    int process();
    void Verify();

    void Append(const char *str, size_t len);

    bool IsKeepAlice() const;

private:
    void ParseQuestLine_();
    void ParseHeadLine_();
    void ParseBodyLine_();

    void ParseHeadLine_(const std::string &Line);
    void ParseCookie_(const std::string &Line);
    int ParseFile_();
    int ParseUser_();

    void Login_();
    void AddCookie_();

    Buffer RecvMsg_;

    std::string Body_;
    std::unordered_map<std::string, std::string> MsgBody_;

    unsigned int BodyLen;

    std::string recvFileName;

    std::string Method, Resource, Version;

    // 0:全部正确 1:密码错误 2:用户不存在 -1:格式错误
    int LoginStatus_;

    HttpMessage *Message_;
};