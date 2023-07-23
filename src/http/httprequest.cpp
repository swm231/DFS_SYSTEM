#include "httprequest.h"

const std::unordered_set<std::string> Request::DEFAULT_HTML{
        "/index", "/register", "/login", "/welcome", "/public", "/private", "/namerr", "/pwderr"};

void HttpRequest::Init(int fd){
    fd_ = fd;
    code_ = LoginStatus_ = -1;
    Method = Resource = Version = recvFileName = resDir_ = action_ = Body_ = username_ = "";
    HeadStatus = HANDLE_INIT;
    BodySatus = EMPTY_TYPE;
    FileStatus = FILE_BEGIN;
    MsgHeader.clear();
}
void HttpRequest::Close(){}

// 0:解析正确 1:继续监听 2:关闭连接
int HttpRequest::process(){
    std::cout << "开始解析" << std::endl;
    char buff[4096];
    while(true){
        int recvLen = recv(fd_, buff, 4096, 0);
        if(recvLen == 0)
            return 2;
        if(recvLen == -1){
            if(errno != EAGAIN)
                return 2;
            return 1;
        }

        RecvMsg.append(buff, recvLen);

        std::string::size_type endIndex;

        // 请求行
        if(HeadStatus == HANDLE_INIT)
            ParseQuestLine_();

        // 首部
        if(HeadStatus == HANDLE_HEAD)
            ParseHeadLine_();

        // 消息体
        if(HeadStatus == HANDLE_BODY){
            if(Method == "GET"){
                HeadStatus = HANDLE_COMPLATE;
                return 0;
            }

            if(Method == "POST"){
                std::string strLine;

                // 文件
                if(MsgHeader["Content-Type"] == "multipart/form-data"){
                    int ret = ParseFile_();
                    if(ret == -1) continue;
                    return ret;
                }
                // 登录 和 注册
                else {
                    int ret = ParseUser_();
                    if(ret == -1) continue;
                    return ret;
                }
            }
        }
    }

    code_ = 404;
    return 0;
}

void HttpRequest::ParseQuestLine_(){
    std::string::size_type endIndex = RecvMsg.find("\r\n");
    if(endIndex == std::string::npos)
        return;
    
    std::string curLine = RecvMsg.substr(0, endIndex + 2);
    RecvMsg.erase(0, endIndex + 2);
    HeadStatus = HANDLE_HEAD;

    std::istringstream lineStream(curLine);
    lineStream >> Method;
    lineStream >> Resource;
    lineStream >> Version;

    std::cout << "请求资源" << Resource << std::endl;

    if(Resource == "/")
        Resource = "/index";
    else{
        std::string::size_type idx = Resource.find('/', 1);
        if(idx == std::string::npos){
            resDir_ = Resource;
            return;
        }

        resDir_ = Resource.substr(0, idx);
        Resource.erase(0, idx);

        idx = Resource.find('/', 1);
        if(idx == std::string::npos){
            code_ = 404;
            return;
        }
        code_ = 302;
        action_ = Resource.substr(0, idx);
        Resource.erase(0, idx);
    }
}

void HttpRequest::ParseHeadLine_(){
    std::string::size_type endIndex;
    while(true){
        endIndex = RecvMsg.find("\r\n");
        if(endIndex == std::string::npos)
            break;
        std::string curLine = RecvMsg.substr(0, endIndex + 2);
        RecvMsg.erase(0, endIndex + 2);

        if(curLine == "\r\n"){
            HeadStatus = HANDLE_BODY;
            if(MsgHeader["Content-Type"] == "multipart/form-data"){
                FileStatus = FILE_BEGIN;
            }
            break;
        }
        addHeaderOpt(curLine);
    }
}

void HttpRequest::ParseBodyLine_(){
    std::string key, value;
    int n = Body_.size(), flag = 0;
    for(int i = 0; i < n; i ++){
        if(Body_[i] == '=')
            flag ^= 1;
        else if(Body_[i] == '&'){
            flag ^= 1;
            MsgBody_[key] = value;
            key = value = "";
        }
        else if(flag)
            value += Body_[i];
        else if(!flag)
            key += Body_[i];
    }
    if(key != "" || value != "")
        MsgBody_[key] = value;
}

int HttpRequest::ParseFile_(){
    std::string::size_type endIndex;
    std::string strLine;

    // 边界首
    if(FileStatus == FILE_BEGIN){
        endIndex = RecvMsg.find("\r\n");
        if(endIndex == std::string::npos)
            return -1;

        strLine = RecvMsg.substr(0, endIndex);
        if(strLine == std::string("--") + MsgHeader["boundary"]){
            FileStatus = FILE_HEAD;
            RecvMsg.erase(0, endIndex + 2);
        }
        else
            return 0;
    }

    // 文件名
    if(FileStatus == FILE_HEAD)
        while(true){
            endIndex = RecvMsg.find("\r\n");
            if(endIndex == std::string::npos)
                break;

            strLine = RecvMsg.substr(0, endIndex + 2);
            RecvMsg.erase(0, endIndex + 2);

            if(strLine == "\r\n"){
                FileStatus = FILE_CONTENT;
                break;
            }

            endIndex = strLine.find("filename");
            if(endIndex == std::string::npos) continue;
            strLine.erase(0, endIndex + std::string("filename=\"").size());
            for(int i = 0; strLine[i] != '\"'; i++)
                recvFileName += strLine[i];
        }

    // 文件内容
    if(FileStatus == FILE_CONTENT){
        std::ofstream ofs("../user_resources/public/" + recvFileName, std::ios::out | std::ios::app | std::ios::binary);
        if(!ofs){
            // log
            return 1;
        }
        while(true){
            int saveLen = RecvMsg.size();
            if(saveLen == 0)
                break;

            endIndex = RecvMsg.find('\r');
            if(endIndex != std::string::npos){
                int endBoundaryLen = MsgHeader["boundary"].size() + 8;
                if(RecvMsg.size() - endIndex >= endBoundaryLen){
                    if(RecvMsg.substr(endIndex, endBoundaryLen) == "\r\n--" + MsgHeader["boundary"] + "--\r\n"){
                        if(endIndex == 0){
                            FileStatus = FILE_COMPLATE;
                            break;
                        }
                        saveLen = endIndex;
                    }
                    else{
                        endIndex = RecvMsg.find('\r', endIndex + 1);
                        if(endIndex != std::string::npos)
                            saveLen = endIndex;
                    }
                }
                else{
                    if(endIndex == 0)
                        break;
                    saveLen = endIndex;
                }
            }
            ofs.write(RecvMsg.c_str(), saveLen);
            RecvMsg.erase(0, saveLen);
        }
        ofs.close();
    }
    // 边界尾
    if(FileStatus == FILE_COMPLATE){
        RecvMsg.erase(0, MsgHeader["boundary"].size() + 8);
        HeadStatus = HANDLE_COMPLATE;
        return 0;
    }
}

int HttpRequest::ParseUser_(){
    if(BodyLen > 0){
        int mn = std::min(BodyLen, static_cast<unsigned int>(RecvMsg.size()));
        Body_ += RecvMsg.substr(0, mn);
        BodyLen -= mn;
    }
    if(BodyLen <= 0){
        ParseBodyLine_();
        Verify_();
        return 0;
    }
    return -1;
}

void HttpRequest::Verify_(){
    std::string name = MsgBody_["username"], password = MsgBody_["password"];
    if(name == "" || password == "")
        return;

    MYSQL *sql;
    SqlConnRAII RAII(&sql);

    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;

    snprintf(order, 256, 
        "SELECT username, password FROM user WHERE username = '%s' LIMIT 1;", name.c_str());
    
    if(mysql_query(sql, order))
        return;
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_field(res);

    MYSQL_ROW row = mysql_fetch_row(res);
    if(row){
        std::string pwd(row[1]);
        if(pwd == password) {
            LoginStatus_ = 0;
            username_ = name;
            Login_();
        }
        else {
            LoginStatus_ = 1;
            Resource = "/pwderr";
        }
    }
    else{
        LoginStatus_ = 2;
        if(Resource == "/login")
            Resource = "/namerr";
        else if(Resource == "/register"){
            bzero(order, 256);
            snprintf(order, 256,
                "INSERT INTO user(username, password) VALUE('%s', '%s');", name.c_str(), password.c_str());
            mysql_query(sql, order);
            username_ = name;
            Login_();
        }
    }
}

void HttpRequest::Login_(){
    Resource = "/welcome";
    if(LoginStatus_ == 2)
        mkdir(("../user_resources/" + username_).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    LoginStatus_ = 0;
}

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg.append(str, len);
}

bool HttpRequest::IsKeepAlice() const{
    if(MsgHeader.count("Connection") == 1)
        return MsgHeader.find("Connection")->second == "keep-alive" && Version == "HTTP/1.1";
    return false;
}