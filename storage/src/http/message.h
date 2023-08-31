#pragma once

#include <assert.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <libconfig.h++>
#include <dirent.h>
#include <arpa/inet.h>
#include <fstream>

#include "../single/encipher.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../buffer/buffer.h"

// 表示信息处理状态
enum MSGSTATUS{
    HANDLE_INIT,
    HANDLE_HEAD,
    HANDLE_BODY,
    HANDLE_COMPLATE,
    HANDLE_ERROR,
};

// 表示文件处理过程
enum FILEMSGESTATUS{
    FILE_BEGIN,
    FILE_HEAD,
    FILE_CONTENT,
    FILE_COMPLATE
};

enum METHOD{
    GET,
    POST,
    METHOD_OTHER,
};

enum PATH{
    PUBLIC,
    PRIVATE,
    PATH_OTHER,
};

enum BEHAVIOR{
    UPLOAD,
    DOWNLOAD,
    DELETE,
    BEHAVIOR_OTHER,
};

enum VERSION{
    _0_9,
    _1_0,
    _1_1,
    _2_0,
    VERSION_OTHER,
};

enum HTML_ENUM{
    _404_, _403_, _400_, _HEAD, _HEAD_, _INDEX, _LOGIN, _LOGOUT, _NAMERR,
     _PWDERR, _PUBLIC, _PRIVATE, _REGISTER, _TITLE, _WELCOME, _LISTEND 
};


class Request{
public:
    Request(){}

public:
    MSGSTATUS HeadStatus;
    FILEMSGESTATUS FileStatus;

    static const std::unordered_map<std::string, BEHAVIOR> DEFAULT_BEHAVIOR;
};

class Response{
public:
    Response(){}

public:
    MSGSTATUS HeadStatus;

    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<HTML_ENUM, const char*> HTML_RESOURCE;
};

class HttpMessage{
public:
    int fd_;
    int SaveErrno;

    HttpMessage(){}
    void Init(){
        Method = METHOD::METHOD_OTHER, Path = PATH::PATH_OTHER, 
        Behavior = BEHAVIOR::BEHAVIOR_OTHER, Version = VERSION::VERSION_OTHER;
        isPrivateUser = false;
        UserName = PassWord = FileName = "";
        MsgHeader.clear();
        code = 200;
    }

    METHOD Method;
    PATH Path;
    BEHAVIOR Behavior;
    VERSION Version;

    bool isPrivateUser;

    int code;
    std::string UserName, PassWord;
    std::string FileName;
    std::string Cookie;

    std::unordered_map<std::string, std::string> MsgHeader;
    
    static char CR[];
    static char CRLF[];
};


class FdNode{
public:
    FdNode();
    FdNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    FdNode(const FdNode& other);
    FdNode(FdNode &&other);
    FdNode &operator=(const FdNode& other);
    virtual ~FdNode();

    virtual void Init();
    virtual void Close() = 0;

    virtual int ReadProcess();
    virtual int WriteProcess();

    uint32_t ip;
    uint16_t port, fd;
    int saveErrno;
    Buffer recvBuff, sendBuff;

    MSGSTATUS status;
    bool isOut;

    static bool InitListenSocket(uint16_t &sockt, int port);

private:
    virtual void ParseHead_() {  }
    virtual void ParseBody_() {  }

};

class TaskNode: public FdNode{
public:
    TaskNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    ~TaskNode();

    void Init();
    void Close();

private:
    void ParseHead_();
    void ParseBody_();

    void DealNEW_(const char*);
    void DealDEL_(const char*);
    void DealUPL_(const char*);

    long long hasSaveLen_, BodyLen_;
    std::ofstream ofs;
};

class TrackerNode : public FdNode{
public:
    TrackerNode():FdNode(){}
    TrackerNode(uint32_t _ip, uint16_t _port, int16_t _fd);
    TrackerNode(TrackerNode &&other);
    TrackerNode &operator=(const TrackerNode &other);
    ~TrackerNode();

    void Close() {  }

    void ReadyTraNEW();
    void ReadyTraKEP();

private:
    void ParseHead_();

    void DealTranew_();
    void DealTrakep_();

    void ReadySYN_(){  }
    void Readysyn_(){  }
};

class StorageNode : public FdNode{
public:
    StorageNode(): FdNode(), fileMsgfd_(-1), hasSentLen_(0){}
    StorageNode(uint32_t _ip, uint16_t _port, uint16_t _fd);
    StorageNode(StorageNode &&other);
    ~StorageNode();

    void Init();
    void Close();

    void ReadyStoNEW();
    void ReadySYN(BEHAVIOR, PATH, const std::string &username, const std::string &filename);

    static std::unordered_map<int, StorageNode*> group;

private:
    int WriteProcess();

    long long hasSentLen_, BodyLen;
    int fileMsgfd_;
};


class ListenHttp: public FdNode{
public:
    ListenHttp(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenHttp(){  }


    void Close(){  }
private:
    int ReadProcess();
    int WriteProcess();
};

class ListenTask: public FdNode{
public:
    ListenTask(uint16_t _port){
        InitListenSocket(fd, _port);
    }
    ~ListenTask(){  }

    void Close(){  }
private:
    int ReadProcess();
    int WriteProcess();
};


class Conf{
public:
    static Conf &Instance(){
        static Conf instance;
        return instance;
    }

    uint32_t bind_addr;
    uint16_t http_port, task_port;
    uint32_t data_capacity, data_used;
    std::string group_name, data_path;
    std::string mysql_host, mysql_user, mysql_pwd, mysql_database;
    std::vector<TrackerNode> tracker;

    Conf(){
        data_used = 0;
    }
    ~Conf(){ }

    bool Parse(){
        libconfig::Config cfg;
        try
        {
            cfg.readFile("../storage.conf");
        }
        catch(const libconfig::FileIOException &fioex)
        {
            LOG_ERROR("[config]I/O error while reading file.")
            return false;
        }
        catch(const libconfig::ParseException &pex)
        {
            LOG_ERROR("[config] Parse error at %s:%s - %s", pex.getFile(), pex.getLine(), pex.getError());
            return false;
        }

        std::string tmp;
        std::string::size_type pos;
        unsigned int uit;

        // group_name
        if(cfg.lookupValue("group_name", group_name) == false){
            LOG_ERROR("[config] Parse group_name error");
            return false;
        }

        // bind_addr
        if(cfg.lookupValue("bind_addr", tmp) == false){
            LOG_ERROR("[config] Parse bind_addr error");
            return false;
        }
        if(ParseIp_(tmp, bind_addr) == false)
            return false;

        // http_port
        if(cfg.lookupValue("http_port", uit) == false){
            LOG_ERROR("[config] Parse http_port error");
            return false;
        }
        if(ParsePort_(uit, http_port) == false)
            return false;

        // task_port
        if(cfg.lookupValue("task_port", uit) == false){
            LOG_ERROR("[config] Parse task_port error");
            return false;
        }
        if(ParsePort_(uit, task_port) == false)
            return false;

        // data_dath
        if(cfg.lookupValue("data_path", data_path) == false){
            LOG_ERROR("[config] Parse data_path error");
            return false;
        }
        if(cfg.lookupValue("data_capacity", data_capacity) == false){
            LOG_ERROR("[config] Parse data_capacity error");
            return false;
        }
        // calc size
        if(getFileSize_(data_path.c_str(), data_used) == false){
            LOG_ERROR("[config] getFileSize error");
            return false;
        }
        // mkdir((data_path + "/" + group_name).c_str(), 777);
        // if(fp = std::fopen((data_path + "/" + group_name + "/.version").c_str(), "r+"))
        //     std::fscanf(fp, "%d", &version);
        // else{
        //     fp = std::fopen((data_path + "/" + group_name + "/.version").c_str(), "w");
        //     std::fprintf(fp, "0");
        //     version = 0;
        // }

        // tracker_server
        for(int i = 0; ; i ++){
            if(cfg.lookupValue((std::string("tracker_server") + char(i + '0')).c_str(), tmp) == false)
                break;
            uint32_t ip; uint16_t port = 33233;
            pos = tmp.find(':');
            if(pos != std::string::npos)
                if(ParsePort_(tmp.substr(pos + 1), port) == false)
                    return false;
            if(ParseIp_(tmp.substr(0, pos), ip) == false)
                return false;
            tracker.emplace_back(ip, port, -1);
        }

        // mysql_host
        if(cfg.lookupValue("mysql_host", mysql_host) == false){
            LOG_ERROR("[config] Parse mysql_host error");
            return false;
        }

        // mysql_user
        if(cfg.lookupValue("mysql_user", mysql_user) == false){
            LOG_ERROR("[config] Parse mysql_user error");
            return false;
        }

        //  mysql_pwd
        if(cfg.lookupValue("mysql_pwd", mysql_pwd) == false){
            LOG_ERROR("[config] Parse mysql_pwd error");
            return false;
        }

        // mysql_database
        if(cfg.lookupValue("mysql_database", mysql_database) == false){
            LOG_ERROR("[config] Parse mysql_database error");
            return false;
        }

        return true;
    }
    
private:
    bool ParsePort_(const std::string &str, uint16_t &port){
        uint32_t port_ = std::stoi(str);
        if(port_ > std::numeric_limits<uint16_t>::max()){
            LOG_ERROR("[config] port is oversize");
            return false;
        }
        port = static_cast<uint16_t>(port_);
        return true;
    }
    bool ParsePort_(const unsigned int port_, uint16_t &port){
        if(port_ > std::numeric_limits<uint16_t>::max()){
            LOG_ERROR("[config] port is oversize");
            return false;
        }
        port = static_cast<uint16_t>(port_);
        return true;
    }

    bool ParseIp_(const std::string &str, uint32_t &ip){
        if(inet_pton(AF_INET, str.c_str(), &ip) == false){
            LOG_ERROR("[config] Invalid IP address format");
            return false;
        }
        ip = ntohl(ip);
        return true;
    }

    bool getFileSize_(const char *path, uint32_t &totalSize){
        DIR *dp;
        struct dirent *entry;
        struct stat statbuf;
        if((dp = opendir(path)) == nullptr){
            LOG_ERROR("[config] can't open data_path");
            return false;
        }

        while((entry = readdir(dp)) != nullptr){
            char subdir[512];
            sprintf(subdir, "%s/%s", path, entry->d_name);
            lstat(subdir, &statbuf);

            if(S_ISDIR(statbuf.st_mode)){
                if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                    continue;
                if(getFileSize_(subdir, totalSize) == false)
                    return false;
            }
            else{
                totalSize += statbuf.st_size;
            }
        }
        closedir(dp);
        return true;
    }
};