#pragma once

#include <fstream>
#include <mutex>

#include "../../message/config.h"
#include "../logformat.h"
#include "../rollbacklog/rollbacklog.h"
#include "../synlog/synlog.h"

class RingLog{
public:
    static RingLog &Instance(){
        static RingLog instance;
        return instance;
    }

    void Init();
    void Close();

    uint64_t WriteRollbackLog(const RollbackLog &RLog);
    uint64_t WriteSynLog(const synLogBody &SLog);

    void FreeLog(uint64_t logAddr);
    
private:
    RingLog();
    ~RingLog();

    void InitLogFile_();
    void RedoTask_();
    void Redo_();

    Page GetFreebody_();

    void ReadPage_(void *page, int offest);
    void WritePage_(void *page, int offest);
    void ReadPage_(Page page);
    void WritePage_(Page page);

    std::fstream logfile;
    std::mutex mtx_;

    ringLogHead head_;
    ringLogBody body_;
};