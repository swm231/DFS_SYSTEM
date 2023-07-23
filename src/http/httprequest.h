#pragma once

#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>

#include "../single/message.h"
#include "../pool/sqlconnraii.h"

class HttpRequest : public Request{
public:
    HttpRequest() : Request() {}
    ~HttpRequest() = default;

    void Init(int fd);
    void Close();

    bool IsKeepAlice() const;

    int process();

    void Append(const char *str, size_t len);

    std::string& Get_resDir(){
        return resDir_;
    }
    std::string& Get_action(){
        return action_;
    }
    int Get_code(){
        return code_;
    }
private:
    void PraseQuestLine(const std::string &line);

    int code_;
    bool isKeepAlive_;
    std::string resDir_;
    std::string action_;

};