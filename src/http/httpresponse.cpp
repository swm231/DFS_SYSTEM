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

HttpResponse::HttpResponse() : code_(-1), path_(""), resource_(""), resPath_(""), isKeepAlive_(false){}

HttpResponse::~HttpResponse(){
    
}

void HttpResponse::Init(const std::string &srcDir, const std::string &resDir, const std::string action, 
            const std::string &resource, bool isKeepAlice, int code){
    resPath_ = resDir;
    path_ = srcDir;
    resource_ = resource;
    isKeepAlive_ = isKeepAlice;
    action_ = action;
    code_ = code;
    HeadStatus = HANDLE_INIT;
    HasSentLen = MsgBodyLen = 0;
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
        printf("数据准备完成！\n");
    }

    while(true){
        long long sentLen = 0;
        if(HeadStatus == HANDLE_HEAD){
            sentLen = HasSentLen;
            sentLen = send(fd_, beforeBodyMsg.c_str() + sentLen, beforeBodyMsgLen - sentLen, 0);
            if(sentLen == -1){
                if(errno != EAGAIN)
                    return 2;
                return 1;
            }
            HasSentLen += sentLen;

            if(HasSentLen >= beforeBodyMsgLen){
                HeadStatus = HANDLE_BODY;
                HasSentLen = 0;
            }
        }

        if(HeadStatus == HANDLE_BODY){
            sentLen = HasSentLen;
            if(BodySatus == TEXT_TYPE)
                sentLen = send(fd_, MsgBody.c_str() + sentLen, MsgBodyLen - sentLen, 0);
            else if(BodySatus == FILE_TYPE)
                sentLen = sendfile(fd_, fileMsgFd, (off_t*)&sentLen, MsgBodyLen - sentLen);
            if(sentLen == -1){
                if(errno != EAGAIN)
                    return 2;
                return 1;
            }
            HasSentLen += sentLen;

            if(HasSentLen >= MsgBodyLen){
                HeadStatus = HANDLE_COMPLATE;
                return 0;
            }
        }
    }
}

void HttpResponse::Parse_(){
    if(action_ == "/delete"){
        if(resPath_ == "/public")
            remove(("../user_resources/" + resPath_ + resource_).c_str());
        // else
        BodySatus = TEXT_TYPE;
        return;
    }
    if(action_ == "/download"){
        if(resPath_ == "/public")
            fileMsgFd = open(("../user_resources" + resPath_ + resource_).c_str(), O_RDONLY);
        // else
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
    beforeBodyMsg = "HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n";
}
void HttpResponse::AddHeader_(){
    // 头部
    beforeBodyMsg += "Content-Type: " + GetFileType_() + "\r\n";
    beforeBodyMsg += "Connection: keep-alive\r\n";
    if(code_ == 302)
        beforeBodyMsg += "Location: /public\r\n";
}
void HttpResponse::AddContent_(){
    // body
    if(BodySatus == FILE_TYPE) {
        beforeBodyMsg += "Content-length: " + std::to_string(fileSata_.st_size) + "\r\n\r\n";
        beforeBodyMsgLen = beforeBodyMsg.size();
        MsgBodyLen = fileSata_.st_size;
        return;
    }
    MsgBody = "";
    GetHtmlPage_();
    MsgBodyLen = MsgBody.size();

    if(BodySatus != EMPTY_TYPE)
        beforeBodyMsg += "Content-length: " + std::to_string(MsgBodyLen) + "\r\n\r\n";
    beforeBodyMsgLen = beforeBodyMsg.size();
}

std::string HttpResponse::GetFileType_(){
    if(BodySatus == FILE_TYPE)
        return "application/octet-stream";
    return "text/html";
}

void HttpResponse::GetHtmlPage_(){
    AddFileStream_("title");
    AddFileStream_("head");
    if(resource_ != "/public"){
        AddFileStream_(resource_);
        return;
    }
    GetFileListPage_();
}

void HttpResponse::AddFileStream_(const std::string &fileName){
    std::ifstream fileListStream("../resources/" + fileName, std::ios::in);
    std::string tempLine;

    while(getline(fileListStream, tempLine))
        MsgBody += tempLine + "\n";
}

void HttpResponse::GetFileListPage_(){
    std::vector<std::string> fileVec;
    if(resPath_ == "/public")
        GetFileVec_("../user_resources/public", fileVec);
    // else 

    std::ifstream fileListStream("../resources/public", std::ios::in);
    std::string tempLine;

    while(true){
        getline(fileListStream, tempLine);
        if(tempLine == "<!-- FileList -->")
            break;
        MsgBody += tempLine + "\n";
    }
    for(auto &filename : fileVec){
        MsgBody += "            <tr><td>" + filename +
                    "</td> <td><a href=\"" + resPath_ + "/download/" + filename +
                    "\">下载</a></td> <td><a href=\"" + resPath_ + "/delete/" + filename +
                    "\" onclick=\"return confirmDelete();\">删除</a></td></tr>" + "\n";
    }

    while(getline(fileListStream, tempLine))
        MsgBody += tempLine + "\n";
}
void HttpResponse::GetFileVec_(const std::string &path, std::vector<std::string> &fileList){
    DIR *dir;
    dir = opendir(path.c_str());
    struct dirent *stdinfo;
    while (1)
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