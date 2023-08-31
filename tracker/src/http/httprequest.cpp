#include "httprequest.h"

const std::unordered_map<std::string, PATH> Request::DEFAULT_PATH{
        {"/index", PATH::ROOT}, {"/register", PATH::REGISTER}, {"/login", PATH::LOGIN}, {"/welcome", PATH::WELCOME},
        {"/public", PATH::PUBLIC}, {"/private", PATH::PRIVATE}, {"/namerr", PATH::NAMERR}, {"/pwderr", PATH::PWDERR},
        {"/get_public_server", PATH::PUBLIC_SERVER}, {"/get_private_server", PATH::PRIVATE_SREVER}};

const std::unordered_map<std::string, BEHAVIOR> Request::DEFAULT_BEHAVIOR{
        {"/delete", BEHAVIOR::DELETE}, {"/upload", BEHAVIOR::UPLOAD}, {"/download", BEHAVIOR::DOWNLOAD}};

void HttpRequest::Init(){
    LOG_DEBUG("[request] fd:%d 请求初始化", Message_->fd_);
    Message_->Init();
    Body_ = Method = Resource = Version = "";
    LoginStatus_ = -1;
    Message_->code = 200;
    HeadStatus = HANDLE_INIT;
    BodySatus = EMPTY_TYPE;

    MsgBody_.clear();
    RecvMsg_.AddHandledAll();
}
void HttpRequest::Close(){}

// 0:解析正确 1:继续监听 2:关闭连接
int HttpRequest::process(){
    LOG_DEBUG("[request] fd:%d 开始解析", Message_->fd_);
    char buff[4096];
    while(true){
        int recvLen = recv(Message_->fd_, buff, 4096, 0);
        // for(int i = 0; i < recvLen; i++)
        //     printf("%c", buff[i]);
        if(recvLen == 0)
            return 2;
        if(recvLen == -1){
            if(errno != EAGAIN)
                return 2;
            return 1;
        }

        RecvMsg_.Append(buff, recvLen);

        std::string::size_type endIndex;

        // 请求行
        if(HeadStatus == HANDLE_INIT)
            ParseQuestLine_();

        // 首部
        if(HeadStatus == HANDLE_HEAD)
            ParseHeadLine_();

        // 验证cookie
        if(LoginStatus_ == -1)
            Verify();

        // 消息体
        if(HeadStatus == HANDLE_BODY){
            if(Message_->Method == METHOD::GET){
                HeadStatus = HANDLE_COMPLATE;
                return 0;
            }

            if(Message_->Method == METHOD::POST){
                // 登录 和 注册
                int ret = ParseUser_();
                if(ret == -1) continue;
                Verify();
                return ret;
            }
        }

        if(HeadStatus == HANDLE_ERROR)
            return 2;
    }
    Message_->code = 404;
    return 0;
}

void HttpRequest::ParseQuestLine_(){
    const char *lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), Message::CRLF, Message::CRLF + 2);
    if(lineEnd == RecvMsg_.BeginWriteConst())
        return;

    std::string curLine(RecvMsg_.Peek(), lineEnd - RecvMsg_.Peek());
    RecvMsg_.AddHandled(lineEnd - RecvMsg_.Peek() + 2);
    HeadStatus = HANDLE_HEAD;

    std::regex patten("^([^ ]*) ([^ ]*) ([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(curLine, subMatch, patten) == 0){
        HeadStatus = HANDLE_ERROR;
        return;
    }
    Method = subMatch[1];
    Resource = subMatch[2];
    Version = subMatch[3];

    LOG_DEBUG("[request] fd:%d 请求资源 %s %s %s", Message_->fd_, Method.c_str(), Resource.c_str(), Version.c_str());

    if(Method == "GET")
        Message_->Method = METHOD::GET;
    else if(Method == "POST")
        Message_->Method = METHOD::POST;
    else 
        Message_->Method = METHOD::METHOD_OTHER;

    if(Resource == "/")
        Message_->Path = PATH::ROOT;
    else if(Resource == "/logout"){
        Message_->isSetCookie = false;
        Message_->Path = PATH::LOGOUT;
    }
    else{
        if(DEFAULT_PATH.count(Resource))
            Message_->Path = DEFAULT_PATH.find(Resource)->second;
        else 
            Message_->Path = PATH::PATH_OTHER;
    }
}

void HttpRequest::ParseHeadLine_(){
    const char *lineEnd;
    while(true){
        lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), Message::CRLF, Message::CRLF + 2);
        if(lineEnd == RecvMsg_.BeginWriteConst())
            break;
        std::string curLine(RecvMsg_.Peek(), lineEnd - RecvMsg_.Peek() + 2);
        RecvMsg_.AddHandled(lineEnd - RecvMsg_.Peek() + 2);

        if(curLine == "\r\n"){
            HeadStatus = HANDLE_BODY;
            break;
        }
        ParseHeadLine_(curLine);
        if(HeadStatus == HANDLE_ERROR)
            return;
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

void HttpRequest::ParseHeadLine_(const std::string &Line){
    std::string key, value;
    std::string::size_type Colon = Line.find(":");
    if(Colon == std::string::npos){
        HeadStatus = HANDLE_ERROR;
        return;
    }
    key = std::string(Line, 0, Colon);
    value = std::string(Line, Colon + 2, Line.size() - (Colon + 2) - 2);

    if(key == "Content-Length")
        BodyLen = std::stoi(value);
    else if(key == "Content-Type"){
        std::string::size_type semIdx = value.find(";");
        if(semIdx != std::string::npos){
            Message_->MsgHeader[key] = value.substr(0, semIdx);
            std::string::size_type eqIdx = value.find("=", semIdx);
            key = value.substr(semIdx + 2, eqIdx - semIdx - 2);
            Message_->MsgHeader[key] = value.substr(eqIdx + 1);
        }
        else
            Message_->MsgHeader[key] = value;
    }
    else if(key == "Cookie")
        Message_->isSetCookie = false,
        ParseCookie_(value);
    else
        Message_->MsgHeader[key] = value;
}

void HttpRequest::ParseCookie_(const std::string &Line){
    bool flag = false;
    for(int i = 0; Line[i]; i++)
        if(Line[i] == '=')
            flag = true;
        else if(flag)
            Message_->PassWord += Line[i];
        else
            Message_->UserName += Line[i];
}

int HttpRequest::ParseUser_(){
    if(BodyLen > 0){
        int mn = std::min(BodyLen, static_cast<unsigned int>(RecvMsg_.UnHandleBytes()));
        Body_ += std::string(RecvMsg_.Peek(), mn);
        RecvMsg_.AddHandled(mn);
        BodyLen -= mn;
    }
    if(BodyLen <= 0){
        ParseBodyLine_();
        return 0;
    }
    return -1;
}

void HttpRequest::Verify(){
    MYSQL *sql;
    SqlConnRAII RAII(&sql);

    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row;

    // 验证cookie
    if(Message_->isSetCookie == false && Message_->UserName.size() && Message_->PassWord.size()){
        LOG_INFO("[request] cookie fd:%d %s %s", Message_->fd_, Message_->UserName.c_str(), Message_->PassWord.c_str());
        snprintf(order, 256,
            "SELECT username FROM user WHERE cookie = '%s' LIMIT 1", Message_->PassWord.c_str());
        mysql_query(sql, order);
        res = mysql_store_result(sql);
        row = mysql_fetch_row(res);
        if(row){
            if(encipher::getMD5(row[0], 4) == Message_->UserName){
                Message_->UserName = row[0];
                LoginStatus_ = 0;
                Message_->isSetCookie = true;
                return;
            }
            else
                Message_->UserName = "";
        }
        Message_->UserName = Message_->PassWord = "";
    }

    // 验证用户名密码
    if(MsgBody_.count("username") == 0 || MsgBody_.count("password") == 0)
        return;

    std::string name = MsgBody_["username"], password = MsgBody_["password"];
    if(name == "" || password == "")
        return;

    LOG_INFO("[request] 用户名密码 fd:%d %s %s", Message_->fd_, name.c_str(), password.c_str());
    snprintf(order, 256, 
        "SELECT username, password FROM user WHERE username = '%s' LIMIT 1;", name.c_str());

    if(mysql_query(sql, order))
        return;
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_field(res);

    row = mysql_fetch_row(res);
    if(row){
        std::string pwd(row[1]);
        if(pwd == password) {
            LoginStatus_ = 0;
            Message_->UserName = name;
            Login_();
        }
        else {
            LoginStatus_ = 1;
            Message_->Path = PATH::PWDERR;
            MsgBody_.erase("username");
        }
    }
    else{
        LoginStatus_ = 2;
        if(Message_->Path == PATH::LOGIN)
            Message_->Path = PATH::NAMERR,
            MsgBody_.erase("username");
        else if(Message_->Path == PATH::REGISTER){
            bzero(order, 256);
            snprintf(order, 256,
                "INSERT INTO user(username, password) VALUE('%s', '%s');", name.c_str(), password.c_str());
            mysql_query(sql, order);
            Message_->UserName = name;
            Login_();
        }
    }
    if(Message_->PassWord != ""){
        bzero(order, 256);
        snprintf(order, 256,
            "UPDATE user SET cookie = '%s' WHERE username = '%s'", Message_->PassWord.c_str(), Message_->UserName.c_str());
        mysql_query(sql, order);
    }
}

void HttpRequest::Login_(){
    LoginStatus_ = 0;
    Message_->isSetCookie = true;
    Message_->Path = PATH::WELCOME;
    AddCookie_();
}

void HttpRequest::AddCookie_(){
    Message_->PassWord = encipher::getCookieValue();
}

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg_.Append(str, len);
}

bool HttpRequest::IsKeepAlice() const{
    if(Message_->MsgHeader.count("Connection") == 1)
        return Message_->MsgHeader.find("Connection")->second == "keep-alive" || Message_->MsgHeader.find("Connection")->second == "Keep-Alive" || Message_->Version == _1_1;
    return false;
}