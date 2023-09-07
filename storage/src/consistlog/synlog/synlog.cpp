#include "synlog.h"

SynLog::SynLog(){}
SynLog::~SynLog(){}

void SynLog::Init(){
    logfile.open((Conf::Instance().data_path + "/log/syn.log").c_str(), 
        std::ios::binary | std::ios::in | std::ios::out);
    if(logfile.is_open() == false){
        mkdir((Conf::Instance().data_path + "/log").c_str(), 777);
        std::ofstream openfile((Conf::Instance().data_path + "/log/syn.log").c_str(), 
            std::ios::binary | std::ios::in | std::ios::out);
        openfile.close();
        logfile.open((Conf::Instance().data_path + "/log/syn.log").c_str(), 
            std::ios::binary | std::ios::in | std::ios::out);
    }

    logfile.seekg(0, std::ios::end);
    fileEnd_ = logfile.tellg();

    bzero(&body_, sizeof body_);
}
void SynLog::Close(){
    if(logfile.is_open())
        logfile.close();
}

uint64_t SynLog::GetPageIdx(){
    std::unique_lock<std::mutex> locker(autoIncrMtx_);
    uint64_t ret = fileEnd_;
    WritePage_(&body_, fileEnd_);
    logfile.seekg(0, std::ios::end);
    fileEnd_ = logfile.tellg();
    return ret;
}

void SynLog::MakeLog(void *page, uint32_t ip, bool isUpload, 
        const std::string &username, const std::string &filename){
    bzero(page, SYNLOG_BASE_SIZE);

    if(isUpload)
        static_cast<synLogBody*>(page)->controlHead |= 1u << SYNLOG_BASE_UPLOAD;
    if(username != "")
        static_cast<synLogBody*>(page)->controlHead |= 1u << SYNLOG_BASE_USERNAME;
    for(int i = 0; i < std::max((int)username.size(), 12); i ++)
        static_cast<synLogBody*>(page)->username[i] = username[i];
    for(int i = 0; i < std::max((int)filename.size(), 12); i ++)
        static_cast<synLogBody*>(page)->filename[i] = filename[i];
}

void SynLog::FinshLog(uint64_t logAddr){
    synLogBody body;
    ReadPage_(&body, logAddr);
    body.controlHead |= 1ull << SYNLOG_BASE_STATUS;
    WritePage_(&body, logAddr);
}

void SynLog::GetPage(synLogBody *body, uint64_t logAddr){
    ReadPage_(body, logAddr);
}

void SynLog::ReadPage_(void *page, int offest){
    logfile.seekg(offest, std::ios::beg);
    logfile.read((char*)page, RINGLOG_BASE_SIZE);
}
void SynLog::WritePage_(void *page, int offest){
    logfile.seekp(offest, std::ios::beg);
    logfile.write((char*)page, RINGLOG_BASE_SIZE);
}
void SynLog::ReadPage_(Page page){
    logfile.seekg(page.second, std::ios::beg);
    logfile.read((char*)page.first, RINGLOG_BASE_SIZE);
}
void SynLog::WritePage_(Page page){
    logfile.seekp(page.second, std::ios::beg);
    logfile.write((char*)page.first, RINGLOG_BASE_SIZE);
}