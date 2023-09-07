#pragma once

#include <iostream>
#include <mutex>
#include <fstream>

#include "../logformat.h"
#include "../../message/config.h"

class SynLog{
public:
    static SynLog &Instance(){
        static SynLog instance;
        return instance;
    }

    void Init();
    void Close();

    uint64_t GetPageIdx();
    void MakeLog(void *page, uint32_t ip, bool isUpload, 
        const std::string &username, const std::string &filename);
    void FinshLog(uint64_t logAddr);

    void GetPage(synLogBody *body, uint64_t logAddr);

private:
    SynLog();
    ~SynLog();

    void ReadPage_(void *page, int offest);
    void WritePage_(void *page, int offest);
    void ReadPage_(Page page);
    void WritePage_(Page page);

    std::fstream logfile;

    synLogBody body_;
    std::streampos fileEnd_;

    // 自增锁
    std::mutex autoIncrMtx_;
};