#include "httpresponse.h"

const std::unordered_map<std::string, std::string> Response::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> Response::CODE_STATUS = {
    { 200, "Ok"},
    { 302, "Found"},
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

void HttpResponse::Init(std::string path, std::string rescouce, bool isKeepAlice, int code){
    resPath_ = "../user_resources/public/";
    path_ = path;
    resource_ = rescouce;
    isKeepAlive_ = isKeepAlice;
    code_ = code;
    HeadStatus = HANDLE_INIT;
    HasSentLen = MsgBodyLen = 0;
}

int HttpResponse::process(){
    if(HeadStatus == HANDLE_INIT){
        Parse_();
        // if(stat((path_ + resource_).c_str(), &fileSata_) < 0 || S_ISDIR(fileSata_.st_mode))
        //     code_ = 404;
        // else if(!(fileSata_.st_mode) & S_IROTH) // 权限问题？
        //     code_ = 403;
        // else if(code_ == -1)
        //     code_ == 200;

        // FindHtml_();
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
                    return 3;
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
            else if(BodySatus == FILE_TYPE){
                sentLen = sendfile(fd_, fileMsgFd, (off_t*)&sentLen, MsgBodyLen - sentLen);
            }

            if(sentLen == -1){
                if(errno != EAGAIN)
                    return 3;
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
    std::string::size_type idx = resource_.find('/', 1);
    if(idx == std::string::npos) {
        BodySatus = TEXT_TYPE;
        return;
    }

    std::string opera = resource_.substr(1, idx - 1);
    if(opera == "delete") {
        resource_.erase(0, idx);
        remove((resPath_ + resource_).c_str());
        if(resource_ != "/css/head.css")
            resource_ = "/public.html";
        BodySatus = TEXT_TYPE;
        return;
    }
    if(opera == "download") {
        resource_.erase(0, idx);
        std::cout << (resPath_ + resource_).c_str() << std::endl;
        fileMsgFd = open((resPath_ + resource_).c_str(), O_RDONLY);
        fstat(fileMsgFd, &fileSata_);
        if(resource_ != "/css/head.css")
            resource_ = "/public.html";
        BodySatus = FILE_TYPE;
        return;
    }
    BodySatus = TEXT_TYPE;
}

void HttpResponse::FindHtml_(){
    if(CODE_PATH.count(code_)){
        resource_ = CODE_PATH.find(code_)->second;
        stat((path_ + resource_).c_str(), &fileSata_);
    }
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

}
void HttpResponse::AddContent_(){
    // body
    if(BodySatus == FILE_TYPE) {
        beforeBodyMsg += "Location: " + std::to_string(fileSata_.st_size - 1) + "\r\n";
        beforeBodyMsg += "Content-length: " + std::to_string(fileSata_.st_size) + "\r\n\r\n";
        beforeBodyMsgLen = beforeBodyMsg.size();
        MsgBodyLen = fileSata_.st_size;
        return;
    }
    MsgBody = "";
    if(resource_ == "/public.html")
        GetFileListPage_();
    else{
        std::ifstream fileStream(path_ + resource_, std::ios::in);
        std::string TempLine;
        while(getline(fileStream, TempLine)){
            MsgBody += TempLine + "\n";
        }
    }
    MsgBodyLen = MsgBody.size();

    if(BodySatus != EMPTY_TYPE)
        beforeBodyMsg += "Content-length: " + std::to_string(MsgBodyLen) + "\r\n\r\n";
    beforeBodyMsgLen = beforeBodyMsg.size();
}

std::string HttpResponse::GetFileType_(){
    if(BodySatus == FILE_TYPE)
        return "application/octet-stream";
    std::string::size_type idx = resource_.find_last_of('.');
    if(idx == std::string::npos)
        return "text/plain";
    std::string suffix = resource_.substr(idx);
    if(SUFFIX_TYPE.count(suffix))
        return SUFFIX_TYPE.find(suffix)->second;

    return "text/plain";
}

void HttpResponse::GetFileListPage_(){
    std::vector<std::string> fileVec;
    GetFileVec_(resPath_ == "" ? "../user_resources/public" : resPath_, fileVec);
    
    std::ifstream fileListStream((std::string("../resources/") + "public.html").c_str(), std::ios::in);
    std::string tempLine;

    while(true){
        getline(fileListStream, tempLine);
        if(tempLine == "<!-- FileList -->")
            break;
        MsgBody += tempLine + "\n";
    }

    for(auto &filename : fileVec){
        MsgBody += "            <tr><td>" + filename +
                    "</td> <td><a href=\"download/" + filename +
                    "\">下载</a></td> <td><a href=\"delete/" + filename +
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