#include "httprequest.h"

const std::unordered_map<std::string, BEHAVIOR> Request::DEFAULT_BEHAVIOR{
        {"/delete", BEHAVIOR::DELETE}, {"/upload", BEHAVIOR::UPLOAD}, {"/download", BEHAVIOR::DOWNLOAD}};

void HttpRequest::Init(){
    LOG_DEBUG("[request] fd:%d 请求初始化", Message_->fd_);
    Message_->Init();
    Body_ = Method = Resource = Version = "";
    LoginStatus_ = -1;
    Message_->code = 200;
    HeadStatus = HANDLE_INIT;
    FileStatus = FILE_BEGIN;

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

        // 记录日志
        if(Message_->Behavior == BEHAVIOR::UPLOAD || Message_->Behavior == BEHAVIOR::DELETE){
            Message_->rollbacklog.MakeOrder(Message_->Behavior == BEHAVIOR::UPLOAD,
                (Message_->Path == PATH::PUBLIC) ? "" : Message_->UserName, Message_->FileName);
            Message_->ringlogAddr_Rollback = RingLog::Instance().WriteRollbackLog(Message_->rollbacklog);
        }

        // 消息体
        if(HeadStatus == HANDLE_BODY){
            if(Message_->Method != METHOD::POST){
                HeadStatus = HANDLE_COMPLATE;
                LOG_DEBUG("[request] 解析完成 return 0");
                return 0;
            }

            if(Message_->Method == METHOD::POST){
                // 文件
                if(Message_->MsgHeader["Content-Type"] == "multipart/form-data"){
                    int ret = ParseFile_();
                    if(ret == -1) continue;
                    return ret;
                }
            }
        }

        if(HeadStatus == HANDLE_ERROR)
            return 2;
    }
    Message_->code = 404;
    return 0;
}

void HttpRequest::ParseQuestLine_(){
    const char *lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
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

    std::string::size_type st_pos = 0, en_pos;
    en_pos = Resource.find('/', st_pos + 1);
    if(en_pos == std::string::npos){
        Message_->Path = PATH::PATH_OTHER;
        Message_->code = 404;
        return;
    }

    if(Resource.substr(st_pos, en_pos - st_pos) == "/public")
        Message_->Path = PATH::PUBLIC;
    else if(Resource.substr(st_pos, en_pos - st_pos) == "/private")
        Message_->Path = PATH::PRIVATE;
    else 
        Message_->Path = PATH::PATH_OTHER;

    st_pos = en_pos;
    en_pos = Resource.find('/', st_pos + 1);
    if(en_pos == std::string::npos){
        Message_->Behavior = BEHAVIOR_OTHER;
        Message_->code = 404;
        return;
    }

    if(DEFAULT_BEHAVIOR.count(Resource.substr(st_pos, en_pos - st_pos)))
        Message_->Behavior = DEFAULT_BEHAVIOR.find(Resource.substr(st_pos, en_pos - st_pos))->second;
    else 
        Message_->Behavior = BEHAVIOR::BEHAVIOR_OTHER;

    Message_->FileName = Resource.substr(en_pos + 1);
    Message_->code = 200;
}

void HttpRequest::ParseHeadLine_(){
    const char *lineEnd;
    while(true){
        lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
        if(lineEnd == RecvMsg_.BeginWriteConst())
            break;
        std::string curLine(RecvMsg_.Peek(), lineEnd - RecvMsg_.Peek() + 2);
        RecvMsg_.AddHandled(lineEnd - RecvMsg_.Peek() + 2);

        if(curLine == "\r\n"){
            HeadStatus = HANDLE_BODY;
            if(Message_->MsgHeader["Content-Type"] == "multipart/form-data"){
                FileStatus = FILE_BEGIN;
            }
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

int HttpRequest::ParseFile_(){
    std::string::size_type endIndex;
    const char *lineEnd;
    std::string strLine;

    // 边界首
    if(FileStatus == FILE_BEGIN){
        lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
        if(lineEnd == RecvMsg_.BeginWriteConst())
            return -1;

        strLine = std::string(RecvMsg_.Peek(), lineEnd - RecvMsg_.Peek());
        if(strLine == std::string("--") + Message_->MsgHeader["boundary"]){
            FileStatus = FILE_HEAD;
            RecvMsg_.AddHandled(lineEnd - RecvMsg_.Peek() + 2);
        }
        else
            return 2;
    }

    // 文件名
    if(FileStatus == FILE_HEAD)
        while(true){
            lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), HttpMessage::CRLF, HttpMessage::CRLF + 2);
            if(lineEnd == RecvMsg_.BeginWriteConst())
                break;

            strLine = std::string(RecvMsg_.Peek(), lineEnd - RecvMsg_.Peek()  + 2);
            RecvMsg_.AddHandled(lineEnd - RecvMsg_.Peek() + 2);

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
        std::string filePath;
        if(Message_->Path == PATH::PUBLIC)
            filePath = Conf::Instance().data_path + "/public/" + recvFileName;
        else if(Message_->Path == PATH::PRIVATE)
            filePath = Conf::Instance().data_path + "/private/" + Message_->UserName + "/" + recvFileName;
        std::ofstream ofs(filePath.c_str(), std::ios::out | std::ios::app | std::ios::binary);


        if(!ofs){
            LOG_ERROR("[request] fd:%d 文件打开失败 %s", Message_->fd_, filePath.c_str());
            return 2;
        }
        while(true){
            int saveLen = RecvMsg_.UnHandleBytes();
            if(saveLen == 0)
                break;

            lineEnd = std::search(RecvMsg_.Peek(), RecvMsg_.BeginWriteConst(), HttpMessage::CR, HttpMessage::CR + 1);
            if(lineEnd != RecvMsg_.BeginWriteConst()){
                int endBoundaryLen = Message_->MsgHeader["boundary"].size() + 8;
                if(RecvMsg_.BeginWriteConst() - lineEnd >= endBoundaryLen){
                    if(std::string(lineEnd, endBoundaryLen) == "\r\n--" + Message_->MsgHeader["boundary"] + "--\r\n"){
                        if(lineEnd == RecvMsg_.Peek()){
                            FileStatus = FILE_COMPLATE;
                            break;
                        }
                        saveLen = lineEnd - RecvMsg_.Peek();
                    }
                    else if(lineEnd + 1 < RecvMsg_.BeginWriteConst()){
                        lineEnd = std::search(lineEnd + 1, RecvMsg_.BeginWriteConst(), HttpMessage::CR, HttpMessage::CR + 1);
                        if(lineEnd != RecvMsg_.BeginWriteConst())
                            saveLen = lineEnd - RecvMsg_.Peek();
                    }
                }
                else{
                    if(lineEnd == RecvMsg_.Peek())
                        break;
                    saveLen = lineEnd - RecvMsg_.Peek();
                }
            }
            ofs.write(RecvMsg_.Peek(), saveLen);
            RecvMsg_.AddHandled(saveLen);
        }
        ofs.close();
    }
    // 边界尾
    if(FileStatus == FILE_COMPLATE){
        RecvMsg_.AddHandled(Message_->MsgHeader["boundary"].size() + 8);
        HeadStatus = HANDLE_COMPLATE;
        RecvMsg_.AddHandledAll();
        LOG_DEBUG("[request] fd:%d 文件处理完成", Message_->fd_);
        return 0;
    }
    return -1;
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
    if(Message_->isPrivateUser == true || Message_->UserName.size() == 0 || Message_->PassWord.size() == 0)
        return;

    MYSQL *sql;
    SqlConnRAII RAII(&sql);

    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_RES *res = nullptr;
    MYSQL_ROW row;

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
            Message_->isPrivateUser = true;
            return;
        }
        else
            Message_->UserName = "";
    }
    Message_->UserName = Message_->PassWord = "";
}