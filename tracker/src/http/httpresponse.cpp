#include "httpresponse.h"

const std::unordered_map<int, std::string> Response::CODE_STATUS = {
    { 200, "Ok"},
    { 302, "Moved Temporarily"},
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<HTML_ENUM, const char*> Response::HTML_RESOURCE = {
    { HTML_ENUM::_400_, _400}, 
    { HTML_ENUM::_403_, _403}, 
    { HTML_ENUM::_404_, _404}, 
    { HTML_ENUM::_HEAD, _head}, 
    { HTML_ENUM::_HEAD_, _head_}, 
    { HTML_ENUM::_INDEX, _index}, 
    { HTML_ENUM::_LOGIN, _login},
    { HTML_ENUM::_LOGOUT, _title},
    { HTML_ENUM::_NAMERR, _namerr},
    { HTML_ENUM::_PWDERR, _pwderr},
    { HTML_ENUM::_PUBLIC, _public},
    { HTML_ENUM::_PRIVATE, _private},
    { HTML_ENUM::_REGISTER, _register},
    { HTML_ENUM::_TITLE, _title},
    { HTML_ENUM::_WELCOME, _welcome},
    { HTML_ENUM::_LISTEND, _listend},
    { HTML_ENUM::_PUBLICJS, _publicjs},
    { HTML_ENUM::_PRIVATEJS, _privatejs},
    { HTML_ENUM::_HTMLEND, _htmlend},
};

HttpResponse::HttpResponse(HttpMessage *Message) : 
        fileMsgFd(-1), Message_(Message){}

HttpResponse::~HttpResponse(){}

void HttpResponse::Init(){
    HeadStatus = HANDLE_INIT;
    HasSentLen = BodyMsgLen = 0;
}
void HttpResponse::Close(){
    if(fileMsgFd != -1){
        close(fileMsgFd);
        fileMsgFd = -1;
    }
}

int HttpResponse::process(){
    if(HeadStatus == HANDLE_INIT){
        Parse_();

        AddStateLine_();

        AddHeader_();
        
        AddContent_();

        HeadStatus = HANDLE_HEAD;
        LOG_DEBUG("[http] fd:%d 数据准备完成", Message_->fd_);
    }

    while(true){
        long long sentLen = 0;
        if(HeadStatus == HANDLE_HEAD){
            Message_->SaveErrno = 0;
            sentLen = beforeBodyMsg.WriteFd(Message_->fd_, &(Message_->SaveErrno));
            if(sentLen == -1){
                if(Message_->SaveErrno != EAGAIN)
                    return 2;
                return 1;
            }

            if(beforeBodyMsg.UnHandleBytes() <= 0)
                HeadStatus = HANDLE_BODY;
        }

        if(HeadStatus == HANDLE_BODY){
            sentLen = HasSentLen;
            if(BodySatus == TEXT_TYPE){
                Message_->SaveErrno = 0;
                sentLen = BodyMsg.WriteFd(Message_->fd_, &(Message_->SaveErrno));
            }
            if(sentLen == -1){
                if(errno != EAGAIN)
                    return 2;
                return 1;
            }
            HasSentLen += sentLen;

            if(HasSentLen >= BodyMsgLen){
                HeadStatus = HANDLE_COMPLATE;
                return 0;
            }
        }
    }
    return 2;
}

void HttpResponse::Parse_(){
    if(Message_->Path == PATH::LOGOUT)
        Message_->code = 302;
    BodySatus = TEXT_TYPE;
}

void HttpResponse::AddStateLine_(){
    // 状态行
    std::string status;
    if(CODE_STATUS.count(Message_->code))
        status = CODE_STATUS.find(Message_->code)->second;
    else{
        Message_->code = 400;
        status = CODE_STATUS.find(Message_->code)->second;
    }
    beforeBodyMsg.Append("HTTP/1.1 " + std::to_string(Message_->code) + " " + status + "\r\n");
}
void HttpResponse::AddHeader_(){
    // 头部
    beforeBodyMsg.Append("Content-Type: " + GetFileType_() + "\r\n");
    beforeBodyMsg.Append("Connection: keep-alive\r\n");
    if(Message_->code == 302){
        if(Message_->Path == PATH::LOGOUT)
            beforeBodyMsg.Append("Location: /\r\n");
        if(Message_->Path == PATH::PUBLIC)
            beforeBodyMsg.Append("Location: /public\r\n");
        if(Message_->Path == PATH::PRIVATE)
            beforeBodyMsg.Append("Location: /private\r\n");
    }
    // if(Message_->Path == PATH::PUBLIC)
    //     HeaderAddPubAddr_();
    // if(Message_->Path == PATH::PRIVATE)
    //     HeaderAddPriAddr_();
    if(Message_->isSetCookie)
    {
        if(Message_->Path == PATH::LOGOUT)
            beforeBodyMsg.Append("Set-Cookie: " + encipher::getMD5(Message_->UserName, 4) + "=" + Message_->PassWord + "; Max-Age=0\r\n");
        else
            beforeBodyMsg.Append("Set-Cookie: " + encipher::getMD5(Message_->UserName, 4) + "=" + Message_->PassWord + "; Max-Age=" + std::to_string(Conf::cookieOut) + "\r\n");
    }
}
void HttpResponse::AddContent_(){
    // body
    // printf("%d\n", Message_->Path);
    if(Message_->Path == PATH::PUBLIC_SERVER || Message_->Path == PATH::PRIVATE_SREVER){
        // beforeBodyMsg.Append("Content-length: " + std::to_string(fileSata_.st_size) + "\r\n\r\n");
        if(Message_->Path == PATH::PUBLIC_SERVER)
            HeaderAddPubAddr_();
        else
            HeaderAddPriAddr_();
        beforeBodyMsg.Append("Content-length: " + std::to_string(BodyMsg.UnHandleBytes()) + "\r\n\r\n");
        return;
    }
    
    BodyMsg.AddHandledAll();
    GetHtmlPage_();
    BodyMsgLen = BodyMsg.UnHandleBytes();

    if(BodySatus != EMPTY_TYPE)
        beforeBodyMsg.Append("Content-length: " + std::to_string(BodyMsgLen) + "\r\n\r\n");
    beforeBodyMsgLen = beforeBodyMsg.UnHandleBytes();
}

std::string HttpResponse::GetFileType_(){
    return "text/html";
}
void HttpResponse::HeaderAddPubAddr_(){
    HeaderAddAddr_("public");
}
void HttpResponse::HeaderAddPriAddr_(){
    std::string groupName = ConsistentHash::Instance().GetGroup(Message_->UserName);
    HeaderAddAddr_(groupName);
}
void HttpResponse::HeaderAddAddr_(const std::string &groupName){
    // TODO 一致性哈希
    if(Message::group[groupName].size() == 0)
        return;
    int rd = rand() % Message::group[groupName].size(), j = 0;
    struct in_addr addr;
    char str[INET_ADDRSTRLEN] = {0};
    for(auto it : Message::group[groupName]){
        if(j == rd){
            BodyMsg.Append("http://");
            addr.s_addr = htonl(Message::storageNode[it]->ip);
            inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
            BodyMsg.Append(str);
            BodyMsg.Append(":");
            BodyMsg.Append(std::to_string(Message::storageNode[it]->HttpPort));
            break;
        }
        j++;
    }
}

void HttpResponse::GetHtmlPage_(){
    AddFileStream_(HTML_ENUM::_TITLE);

    // 没有登陆 或者 要登出了
    if(Message_->UserName == "")
        AddFileStream_(HTML_ENUM::_HEAD);
    else{
        AddFileStream_(HTML_ENUM::_HEAD_);
        BodyMsg.Append("        <div class=\"User\"><ul><li><a class=\"navigation\" href=\"/private\">" + Message_->UserName +
            "</a></li><li><a class=\"navigation\" href=\"/logout\">" + "登出" +
            "</a></li></ul></div></div>");
    }
    if(Message_->UserName == "" && Message_->Path == PATH::PRIVATE)
        Message_->Path = PATH::LOGIN;

    if(Message_->Path != PATH::PUBLIC && Message_->Path != PATH::PRIVATE){
        switch (Message_->Path)
        {
        case PATH::ROOT:
            AddFileStream_(HTML_ENUM::_INDEX); break;
        case PATH::LOGIN:
            AddFileStream_(HTML_ENUM::_LOGIN); break;
        case PATH::REGISTER:
            AddFileStream_(HTML_ENUM::_REGISTER); break;
        case PATH::WELCOME:
            AddFileStream_(HTML_ENUM::_WELCOME); break;
        case PATH::NAMERR:
            AddFileStream_(HTML_ENUM::_NAMERR); break;
        case PATH::PWDERR:
            AddFileStream_(HTML_ENUM::_PWDERR); break;
        default:
            AddFileStream_(HTML_ENUM::_404_); break;
        }
        return;
    }
    GetFileListPage_();
}

void HttpResponse::GetFileListPage_(){
    std::vector<std::string> fileVec;
    if(Message_->Path == PATH::PUBLIC)
        GetPubFileVec_(fileVec),
        AddFileStream_(HTML_ENUM::_PUBLIC);
    else if(Message_->Path == PATH::PRIVATE)
        GetPriFileVec_(fileVec),
        AddFileStream_(HTML_ENUM::_PRIVATE);

    for(auto &filename : fileVec){
        BodyMsg.Append(" <tr><td>" + filename + "</td> <td> <a href=\"#\" onclick=\"DownLoad('"
                + filename + "')\">下载</a>" + "</td><td> <a href=\"#\" onclick=\"Delete('"
                + filename + "')\">删除</a> </td> </tr>\n");
    }

    AddFileStream_(HTML_ENUM::_LISTEND);
    if(Message_->Path == PATH::PUBLIC)
        AddFileStream_(HTML_ENUM::_PUBLICJS);
    else if(Message_->Path == PATH::PRIVATE)
        AddFileStream_(HTML_ENUM::_PRIVATEJS);
    AddFileStream_(HTML_ENUM::_HTMLEND);
}
void HttpResponse::GetPubFileVec_(std::vector<std::string> &fileList){
    MYSQL *sql;
    SqlConnRAII RAII(&sql);
    char order[256] = {0};
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row;
    snprintf(order, 256, "SELECT filename FROM public");
    mysql_query(sql, order);
    res = mysql_store_result(sql);
    while(row = mysql_fetch_row(res))
        fileList.push_back(row[0]);
}
void HttpResponse::GetPriFileVec_(std::vector<std::string> &fileList){
    MYSQL *sql;
    SqlConnRAII RAII(&sql);
    char order[256] = {0};
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row;
    snprintf(order, 256, "SELECT filename FROM private WHERE username = '%s'", Message_->UserName.c_str());
    mysql_query(sql, order);
    res = mysql_store_result(sql);
    while(row = mysql_fetch_row(res))
        fileList.push_back(row[0]);

}
void HttpResponse::AddFileStream_(HTML_ENUM html_enum){
    if(HTML_RESOURCE.count(html_enum) == 0)
        return;
    BodyMsg.Append(HTML_RESOURCE.find(html_enum)->second);
    BodyMsg.Append("\n");
}