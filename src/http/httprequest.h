#pragma once

#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>
#include <cstring>

#include "message.h"
#include "../pool/sqlconnraii.h"

class HttpRequest : public Request{
public:
    HttpRequest() : Request() {}
    ~HttpRequest() = default;

    void Init(int fd);
    void Close();

    bool IsKeepAlice() const;

    int process();
    void Verify();

    void Append(const char *str, size_t len);

    std::string& Get_resDir(){
        return resDir_;
    }
    std::string& Get_action(){
        return action_;
    }
    std::string& Get_username(){
        return username_;
    }
    std::string& Get_cookie(){
        return cookie_;
    }
    int Get_code(){
        return code_;
    }
    int isSetCookie(){
        return isSetCookie_;
    }
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

    int code_;
    bool isKeepAlive_;
    std::string resDir_;
    std::string action_;

    // 0:全部正确 1:密码错误 2:用户不存在 -1:格式错误
    int LoginStatus_;
    std::string username_;
    std::string cookie_key_;
    std::string cookie_;
    int isSetCookie_;

    std::string Body_;
    std::unordered_map<std::string, std::string> MsgBody_;
};