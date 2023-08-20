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
};

extern int _cookieOut;

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

    LOG_DEBUG("before Sent_");
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
            else if(BodySatus == FILE_TYPE)
                sentLen = sendfile(Message_->fd_, fileMsgFd, (off_t*)&sentLen, BodyMsgLen - sentLen);
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
    if(Message_->Behavior == BEHAVIOR::DELETE){
        if(Message_->Path == PATH::PUBLIC)
            remove(("../user_resources/public" + Message_->FileName).c_str());
        else if(Message_->Path == PATH::PRIVATE){
            if(Message_->UserName == "")
                Message_->Path = PATH::LOGIN;
            else 
                remove(("../user_resources/private/" + Message_->UserName + Message_->FileName).c_str());
        }
        BodySatus = TEXT_TYPE;
        if(Message_->UserName != "")
            Message_->code = 302;
        return;
    }
    if(Message_->Behavior == BEHAVIOR::DOWNLOAD){
        if(Message_->Path == PATH::PUBLIC)
            fileMsgFd = open(("../user_resources/public" + Message_->FileName).c_str(), O_RDONLY);
        else if(Message_->Path == PATH::PRIVATE){
            if(Message_->UserName == ""){
                Message_->Path == PATH::LOGIN;
                BodySatus = TEXT_TYPE;
                Message_->code = 200;
                return;
            }
            else fileMsgFd = open(("../user_resources/private/" + Message_->UserName + Message_->FileName).c_str(), O_RDONLY);
        }
        fstat(fileMsgFd, &fileSata_);
        BodySatus = FILE_TYPE;
        Message_->code = 200;
        return;
    }
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
    if(Message_->isSetCookie)
    {
        if(Message_->Path == PATH::LOGOUT)
            beforeBodyMsg.Append("Set-Cookie: " + encipher::getMD5(Message_->UserName, 4) + "=" + Message_->PassWord + "; Max-Age=0\r\n");
        else
            beforeBodyMsg.Append("Set-Cookie: " + encipher::getMD5(Message_->UserName, 4) + "=" + Message_->PassWord + "; Max-Age=" + std::to_string(CookieOut) + "\r\n");
    }
}
void HttpResponse::AddContent_(){
    // body
    if(BodySatus == FILE_TYPE) {
        beforeBodyMsg.Append("Content-length: " + std::to_string(fileSata_.st_size) + "\r\n\r\n");
        beforeBodyMsgLen = beforeBodyMsg.UnHandleBytes();
        BodyMsgLen = fileSata_.st_size;
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
    if(BodySatus == FILE_TYPE)
        return "application/octet-stream";
    return "text/html";
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
        GetFileVec_("../user_resources/public", fileVec),
        AddFileStream_(HTML_ENUM::_PUBLIC);
    else if(Message_->Path == PATH::PRIVATE)
        GetFileVec_("../user_resources/private/" + Message_->UserName, fileVec),
        AddFileStream_(HTML_ENUM::_PRIVATE);

    for(auto &filename : fileVec){
        BodyMsg.Append("            <tr><td>" + filename +
                    "</td> <td><a href=\"" + (Message_->Path == PATH::PRIVATE ? "/private" : "/public") + "/download/" 
                    + filename + "\">下载</a></td> <td><a href=\"" + (Message_->Path == PATH::PRIVATE ? "/private" : "/public") 
                    + "/delete/" + filename + "\" onclick=\"return confirmDelete();\">删除</a></td></tr>" + "\n");
    }

    AddFileStream_(HTML_ENUM::_LISTEND);
}
void HttpResponse::GetFileVec_(const std::string &path, std::vector<std::string> &fileList){
    DIR *dir;
    dir = opendir(path.c_str());
    struct dirent *stdinfo;
    while(true)
    {
        // 获取文件夹中的一个文件
        stdinfo = readdir(dir);
        if (stdinfo == nullptr){
            break;
        }
        fileList.push_back(stdinfo->d_name);
        if(fileList.back() == "." || fileList.back() == ".."){
            fileList.pop_back();
        }
    }
    closedir(dir);
}

void HttpResponse::AddFileStream_(HTML_ENUM html_enum){
    if(HTML_RESOURCE.count(html_enum) == 0)
        return;
    BodyMsg.Append(HTML_RESOURCE.find(html_enum)->second);
    BodyMsg.Append("\n");
}