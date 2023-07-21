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
    path_ = path;
    resource_ = rescouce;
    isKeepAlive_ = isKeepAlice;
    code_ = code;
}

void HttpResponse::MaskeResponse(){
    if(stat((path_ + resource_).c_str(), &fileSata_) < 0 || S_ISDIR(fileSata_.st_mode))
        code_ = 404;
    else if(!(fileSata_.st_mode) & S_IROTH) // 权限问题？
        code_ = 403;
    else if(code_ == -1)
        code_ == 200;

    FindHtml_();
    AddStateLine_();
    AddHeader_();
    AddContent_();
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

    // body长度
    if(MsgBody != "")
        beforeBodyMsg += "Content-length: " + std::to_string(MsgBody.size()) + "\r\n\r\n";
    beforeBodyMsgLen = beforeBodyMsg.size();
    HeadStatus = HANDLE_HEAD;

    printf("数据准备完成！\n");
}

std::string HttpResponse::GetFileType_(){
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