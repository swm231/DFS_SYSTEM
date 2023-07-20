#include "httprequest.h"

const std::unordered_set<std::string> Request::DEFAULT_HTML{
        "/index", "/register", "/login", "/welcome", "/public"};

void HttpRequest::Init(){
    Method = Resource = Version = recvFileName = "";
    HeadStatus = HANDLE_INIT;
    MsgHeader.clear();
    ContentLength = MsgBodyRecvLen = 0;
}

// 0:解析正确 1:首部错误 2:重定向 3:文件未找到
int HttpRequest::parse(){
    std::cout << "开始解析" << std::endl;
    std::cout << RecvMsg << std::endl;

    std::string::size_type endIndex;

    // 请求行
    endIndex = RecvMsg.find("\r\n");
    setRequestLine(RecvMsg.substr(0, endIndex + 2));
    RecvMsg.erase(0, endIndex + 2);
    
    // 首部
    while(1){
        endIndex = RecvMsg.find("\r\n");
        std::string curLine = RecvMsg.substr(0, endIndex + 2);
        RecvMsg.erase(0, endIndex + 2);
        if(curLine == "\r\n"){
            // if(MsgHeader["Content-Type"] == "multipart/form-data")
            //     BodySatus = FIILE_TYPE;
            break;
        }
        addHeaderOpt(curLine);
    }
    
    // 消息体
    if(Method == "POST"){
        std::string strLine;
        // 文件
        if(MsgHeader["Content-Type"] == "multipart/form-data"){
            // 边界首
            endIndex = RecvMsg.find("\r\n");
            strLine = RecvMsg.substr(0, endIndex);
            if(strLine == "--" + MsgHeader["boundary"])
                RecvMsg.erase(0, endIndex + 2);
            else
                return 2;

            // 文件名
            while(true){
                endIndex = RecvMsg.find("\r\n");
                strLine = RecvMsg.substr(0, endIndex + 2);
                RecvMsg.erase(0, endIndex + 2);

                if(strLine == "\r\n")
                    break;
                
                endIndex = strLine.find("filename");
                if(endIndex == std::string::npos) continue;
                strLine.erase(0, endIndex + std::string("filename=\"").size());
                for(int i = 0; strLine[i] != '\"'; i++)
                    recvFileName += strLine[i];
            }
            std::cout << "FILENAME::" << recvFileName << std::endl;
            // 文件内容
            std::ofstream ofs("../user_resources/public/" + recvFileName, std::ios::out | std::ios::app | std::ios::binary);
            if(!ofs){
                // log
            }
            while(true){
                // 如果没有接收完，把所有的数据写入
                int saveLen = RecvMsg.size();
                if(saveLen == 0)
                    break;
                endIndex = RecvMsg.find('\r');
                int endBoundaryLen = MsgHeader["boundary"].size() + 8;
                if(RecvMsg.size() - endIndex > 8){
                    if(RecvMsg.substr(endIndex, endBoundaryLen) == "\r\n--" + MsgHeader["boundary"] + "--\r\n"){
                        if(endIndex == 0)
                            break;
                        saveLen = endIndex;
                    }
                    else{
                        endIndex = RecvMsg.find('\r', endIndex + 1);
                        if(endIndex != std::string::npos)
                            saveLen = endIndex;
                    }
                }
                else
                    saveLen = endIndex;
                ofs.write(RecvMsg.c_str(), saveLen);
                RecvMsg.erase(0, saveLen);
            }
            ofs.close();

            // 边界尾

            return 2;
        }
        else return 2;
    }
    
    std::cout << "解析完成，请求资源:" << Resource << std::endl;

    return 0;
}

void HttpRequest::Append(const char *str, size_t len){
    RecvMsg.append(str, len);
}

bool HttpRequest::IsKeepAlice() const{
    if(MsgHeader.count("Connection") == 1)
        return MsgHeader.find("Connection")->second == "keep-alive" && Version == "1.1";
    return false;
}