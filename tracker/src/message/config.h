#pragma once

#include <iostream>
#include <libconfig.h++>
#include <arpa/inet.h>

#include "../log/log.h"

class Conf{
public:
    static Conf &Instance(){
        static Conf instance;
        return instance;
    }

    static uint32_t bind_addr, cookieOut, timeOut;
    static uint16_t http_port, task_port;
    static std::string mysql_host, mysql_user, mysql_pwd, mysql_database;

    Conf(){}
    ~Conf(){}

    bool Parse(){
        libconfig::Config cfg;
        try
        {
            cfg.readFile("../tracker.conf");
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
        unsigned int uit;

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
        
        // cookieOut
        if(cfg.lookupValue("cookieOut", cookieOut) == false){
            LOG_ERROR("[config] Parse cookieOut error");
            return false;
        }

        // cookieOut
        if(cfg.lookupValue("timeOut", timeOut) == false){
            LOG_ERROR("[config] Parse timeOut error");
            return false;
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
    bool ParseIp_(const std::string &str, uint32_t &ip){
        struct in_addr addr;
        if(inet_pton(AF_INET, str.c_str(), &(addr.s_addr)) == false){
            LOG_ERROR("[config] Invalid IP address format");
            return false;
        }
        ip = ntohl(addr.s_addr);
        return true;
    }
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
};