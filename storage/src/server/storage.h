#pragma once

#include <libconfig.h++>
#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <string>
#include <dirent.h>

#include "../log/log.h"
#include "../pool/threadpool.h"
#include "../single/epoll.h"

class TrackerNode{
public:
    TrackerNode(){}
    TrackerNode(uint32_t _ip, uint16_t _port, int16_t _fd);
    TrackerNode(TrackerNode &&other);

    void Close();

    void DealWrite();

    uint32_t ip;
    uint16_t port;
    int16_t fd;
    int err;
    Buffer recvBuff, sendBuff;

    void ReadyNew();

private:
    void OnRead_();
    void OnWrite_();

    int ReadProcess_();
    int WriteProcess_();
};

class StorageConf{
public:
    StorageConf(){
        data_used = 0;
    }
    ~StorageConf(){}

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
        
        // data
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
            tracker.emplace_back(TrackerNode(ip, port, -1));
        }

        return true;
    }
// private:
    static std::string group_name, data_path;
    static uint32_t bind_addr;
    static uint16_t http_port, task_port;
    static uint32_t data_capacity, data_used;
    std::vector<TrackerNode> tracker;

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
