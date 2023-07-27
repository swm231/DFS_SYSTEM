#include "httpresponse.h"

const std::unordered_map<int, std::string> Response::CODE_STATUS = {
    { 200, "Ok"},
    { 302, "Moved Temporarily"},
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> Response::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

extern int _cookieOut;

HttpResponse::HttpResponse() : code_(-1), path_(""), resource_(""), resPath_(""), isKeepAlive_(false){}

HttpResponse::~HttpResponse(){}

void HttpResponse::Init(const std::string &srcDir, const std::string &resDir, const std::string action, const std::string &resource, 
        const std::string &username, int isSetCookie, const std::string &cookie, bool isKeepAlice, int code){
    path_ = srcDir;
    resPath_ = resDir;
    action_ = action;
    resource_ = resource;
    username_ = username;
    isSetCookie_ = isSetCookie;
    cookie_ = cookie;
    isKeepAlive_ = isKeepAlice;
    code_ = code;
    HeadStatus = HANDLE_INIT;
    HasSentLen = BodyMsgLen = 0;
}
void HttpResponse::Close(){
    if(fileMsgFd != -1)
        close(fileMsgFd);
}

int HttpResponse::process(){
    if(HeadStatus == HANDLE_INIT){
        Parse_();
        AddStateLine_();
        AddHeader_();
        AddContent_();

        HeadStatus = HANDLE_HEAD;
        LOG_DEBUG("[http] fd:%d 数据准备完成", fd_);
    }

    while(true){
        long long sentLen = 0;
        if(HeadStatus == HANDLE_HEAD){
            SaveErrno = 0;
            sentLen = beforeBodyMsg.WriteFd(fd_, &SaveErrno);
            if(sentLen == -1){
                if(SaveErrno != EAGAIN)
                    return 2;
                return 1;
            }

            if(beforeBodyMsg.UnHandleBytes() <= 0)
                HeadStatus = HANDLE_BODY;
        }

        if(HeadStatus == HANDLE_BODY){
            sentLen = HasSentLen;
            if(BodySatus == TEXT_TYPE){
                SaveErrno = 0;
                sentLen = BodyMsg.WriteFd(fd_, &SaveErrno);
            }
            else if(BodySatus == FILE_TYPE)
                sentLen = sendfile(fd_, fileMsgFd, (off_t*)&sentLen, BodyMsgLen - sentLen);
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
}

void HttpResponse::Parse_(){
    if(action_ == "/delete"){
        if(resPath_ == "/public")
            remove(("../user_resources" + resPath_ + resource_).c_str());
        else if(resPath_ == "/private"){
            if(username_ == "")
                resource_ = "/login";
            else 
                remove(("../user_resources" + resPath_ + "/" + username_ + resource_).c_str());
        }
        BodySatus = TEXT_TYPE;
        if(username_ != "")
            code_ = 302;
        return;
    }
    if(action_ == "/download"){
        if(resPath_ == "/public")
            fileMsgFd = open(("../user_resources" + resPath_ + resource_).c_str(), O_RDONLY);
        else if(resPath_ == "/private"){
            if(username_ == ""){
                resource_ = "/login";
                BodySatus = TEXT_TYPE;
                code_ = 200;
                return;
            }
            else fileMsgFd = open(("../user_resources" + resPath_ + "/" + username_ + resource_).c_str(), O_RDONLY);
        }
        fstat(fileMsgFd, &fileSata_);
        BodySatus = FILE_TYPE;
        code_ = 200;
        return;
    }
    BodySatus = TEXT_TYPE;
}

void HttpResponse::AddStateLine_(){
    // 状态行
    std::string status;
    if(CODE_STATUS.count(code_))
        status = CODE_STATUS.find(code_)->second;
    else{
        code_ = 400;
        status = CODE_STATUS.find(code_)->second;
    }
    beforeBodyMsg.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}
void HttpResponse::AddHeader_(){
    // 头部
    beforeBodyMsg.Append("Content-Type: " + GetFileType_() + "\r\n");
    beforeBodyMsg.Append("Connection: keep-alive\r\n");
    if(code_ == 302)
        beforeBodyMsg.Append("Location: " + resPath_ + "\r\n");
    if(isSetCookie_ == 1)
        beforeBodyMsg.Append("Set-Cookie: " + encipher::getMD5(username_, 4) + "=" + cookie_ + "; Max-Age=" + std::to_string(CookieOut) + "\r\n");
    else if(isSetCookie_ == -1){
        beforeBodyMsg.Append("Set-Cookie: " + encipher::getMD5(username_, 4) + "=" + cookie_ + "; Max-Age=0\r\n");
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
    AddFileStream_("title");

    // 没有登陆 或者 要登出了
    if(username_ == "" || isSetCookie_ == -1)
        AddFileStream_("head");
    else{
        AddFileStream_("head_");
        BodyMsg.Append("        <div class=\"User\"><ul><li><a class=\"navigation\" href=\"/private\">" + username_ +
            "</a></li><li><a class=\"navigation\" href=\"/logout\">" + "登出" +
            "</a></li></ul></div></div>");
    }
    if(username_ == "" && resource_ == "/private")
        resource_ = "/login";

    if(resource_ != "/public" && resource_ != "/private"){
        AddFileStream_(resource_);
        return;
    }
    GetFileListPage_();
}
void HttpResponse::AddFileStream_(const std::string &fileName){
    std::ifstream fileListStream("../resources/" + fileName, std::ios::in);
    std::string tempLine;

    while(getline(fileListStream, tempLine))
        BodyMsg.Append(tempLine + "\n");
}

void HttpResponse::GetFileListPage_(){
    std::vector<std::string> fileVec;
    std::ifstream fileListStream;
    if(resPath_ == "/public")
        GetFileVec_("../user_resources/public", fileVec),
        fileListStream.open("../resources/public", std::ios::in);
    else if(resPath_ == "/private")
        GetFileVec_("../user_resources/private/" + username_, fileVec),
        fileListStream.open("../resources/private", std::ios::in);

    std::string tempLine;

    while(true){
        getline(fileListStream, tempLine);
        if(tempLine == "<!-- FileList -->")
            break;
        BodyMsg.Append(tempLine + "\n");
    }
    for(auto &filename : fileVec){
        BodyMsg.Append("            <tr><td>" + filename +
                    "</td> <td><a href=\"" + resPath_ + "/download/" + filename +
                    "\">下载</a></td> <td><a href=\"" + resPath_ + "/delete/" + filename +
                    "\" onclick=\"return confirmDelete();\">删除</a></td></tr>" + "\n");
    }

    while(getline(fileListStream, tempLine))
        BodyMsg.Append(tempLine + "\n");
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
}