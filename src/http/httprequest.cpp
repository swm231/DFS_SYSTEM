#include "httprequest.h"

const std::unordered_set<std::string> Request::DEFAULT_HTML{
        "/index", "/register", "/login", "/welcome", "/public", "/private"};

void HttpRequest::Init(int fd){
    fd_ = fd;
    code_ = -1;
    Method = Resource = Version = recvFileName = resDir_ = action_ = "";
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
        if(HeadStatus == HANDLE_INIT){
            endIndex = RecvMsg.find("\r\n");
            if(endIndex == std::string::npos)
                continue;
            PraseQuestLine(RecvMsg.substr(0, endIndex + 2));
            RecvMsg.erase(0, endIndex + 2);
            HeadStatus = HANDLE_HEAD;
        }

        // 首部
        if(HeadStatus == HANDLE_HEAD)
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
                    // 边界首
                    if(FileStatus == FILE_BEGIN){
                        endIndex = RecvMsg.find("\r\n");
                        if(endIndex == std::string::npos)
                            continue;

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
                // 其他
                else {
                    HeadStatus = HANDLE_COMPLATE;
                    return 0;
                }
            }
        }
    }

    code_ = 404;
    return 0;
}

void HttpRequest::PraseQuestLine(const std::string &line){
    std::istringstream lineStream(line);
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

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg.append(str, len);
}

bool HttpRequest::IsKeepAlice() const{
    if(MsgHeader.count("Connection") == 1)
        return MsgHeader.find("Connection")->second == "keep-alive" && Version == "HTTP/1.1";
    return false;
}