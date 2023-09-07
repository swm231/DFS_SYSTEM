#include "ringlog.h"
#include "../../message/node.h"
#include "../../single/epoll.h"


RingLog::RingLog(){}
RingLog::~RingLog(){ Close(); }

void RingLog::Init(){
    logfile.open((Conf::Instance().data_path + "/log/ring.log").c_str(), std::ios::binary | std::ios::in | std::ios::out);
    if(logfile.is_open() == false){
        mkdir((Conf::Instance().data_path + "/log").c_str(), 777);
        std::ofstream openfile((Conf::Instance().data_path + "/log/ring.log").c_str(), std::ios::out);
        openfile.close();
        logfile.open((Conf::Instance().data_path + "/log/ring.log").c_str(), std::ios::binary | std::ios::in | std::ios::out);
    }

    logfile.seekg(0, std::ios::end);
    std::streampos fileSize = logfile.tellg();

    if(fileSize == 0)
        InitLogFile_();
    
    logfile.seekg(0, std::ios::beg);
    logfile.read((char*)&head_, sizeof head_);

    if(head_.todoListNum != 0)
        RedoTask_();
}
void RingLog::Close(){
    if(logfile.is_open() == true)
        logfile.close();
}

uint64_t RingLog::WriteRollbackLog(const RollbackLog &RLog){
    Page newPage = GetFreebody_();
    bzero(newPage.first, RINGLOG_MEST_SIZE);

    static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_ROLLBACK;
    if(RLog.isUpload)
        static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_UPLOAD;
    for(int i = 0; i < std::max((int)RLog.username.size(), 12); i ++){
        static_cast<ringLogBody*>(newPage.first)->username[i] = RLog.username[i];
        if(i != 11)
            static_cast<ringLogBody*>(newPage.first)->username[i + 1] = '\0';
    }
    for(int i = 0; i < std::max((int)RLog.filename.size(), 12); i ++){
        static_cast<ringLogBody*>(newPage.first)->filename[i] = RLog.filename[i];
        if(i != 11)
            static_cast<ringLogBody*>(newPage.first)->filename[i + 1] = '\0';
    }
    
    WritePage_(newPage);
    delete static_cast<ringLogBody*>(newPage.first);
    return newPage.second;
}
uint64_t RingLog::WriteSynLog(const synLogBody &SLog){
    Page newPage = GetFreebody_();
    bzero(newPage.first, RINGLOG_MEST_SIZE);

    static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_SYN;
    static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_SYNADDR;
    static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_INITIATIVE;
    if(SLog.controlHead & (1u << SYNLOG_BASE_UPLOAD))
        static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_UPLOAD;
    if(SLog.controlHead & (1u << SYNLOG_BASE_USERNAME))
        static_cast<ringLogBody*>(newPage.first)->status |= 1ull << RINGLOG_HEAD_USERNAME;
    for(int i = 0; i < 12; ++i)
        static_cast<ringLogBody*>(newPage.first)->username[i] = SLog.username[i],
        static_cast<ringLogBody*>(newPage.first)->filename[i] = SLog.filename[i];

    WritePage_(newPage);
    delete static_cast<ringLogBody*>(newPage.first);
    return newPage.second;
}

void RingLog::FreeLog(uint64_t logAddr){
    std::unique_lock<std::mutex> locker(mtx_);

    ringLogBody curBody;
    ReadPage_(&curBody, logAddr);

    // 从todoList中去掉
    if(head_.todoList == logAddr){
        if(head_.freeListNum == 1)
            head_.todoList = 0ull;
        else{
            ReadPage_(&body_, curBody.nxt);
            body_.pre = curBody.pre;
            WritePage_(&body_, curBody.nxt);
            head_.todoList = curBody.nxt;
        }
    }
    else{
        ReadPage_(&body_, curBody.nxt);
        body_.pre = curBody.pre;
        WritePage_(&body_, curBody.nxt);

        ReadPage_(&body_, curBody.pre);
        body_.nxt = curBody.nxt;
        WritePage_(&body_, curBody.pre);
    }
    head_.freeListNum --;

    // 加入到freeList
    ReadPage_(&body_, head_.freeListEnd);
    body_.nxt = logAddr;
    WritePage_(&body_, head_.freeListEnd);

    bzero(&curBody, RINGLOG_BASE_SIZE);
    curBody.pre = head_.freeListEnd;
    head_.freeListEnd = logAddr;
    
    WritePage_(&curBody, logAddr);
    head_.freeListEnd = logAddr;
    head_.freeListNum ++;

    WritePage_(&head_, RINGLOG_BASE_SIZE);
}

Page RingLog::GetFreebody_(){
    std::unique_lock<std::mutex> locker(mtx_);
    
    ringLogBody *bodyPtr = new ringLogBody();
    ReadPage_(bodyPtr, head_.freeList);

    uint64_t ret = head_.freeList;
    head_.freeList = bodyPtr->nxt;
    bodyPtr->pre = bodyPtr->nxt;
    bodyPtr->nxt = head_.todoList;
    head_.todoList = ret;

    WritePage_(&head_, 0);

    return {bodyPtr, ret};
}

void RingLog::ReadPage_(void *page, int offest){
    logfile.seekg(offest, std::ios::beg);
    logfile.read((char*)page, RINGLOG_BASE_SIZE);
}
void RingLog::WritePage_(void *page, int offest){
    logfile.seekp(offest, std::ios::beg);
    logfile.write((char*)page, RINGLOG_BASE_SIZE);
}
void RingLog::ReadPage_(Page page){
    logfile.seekg(page.second, std::ios::beg);
    logfile.read((char*)page.first, RINGLOG_BASE_SIZE);
}
void RingLog::WritePage_(Page page){
    logfile.seekp(page.second, std::ios::beg);
    logfile.write((char*)page.first, RINGLOG_BASE_SIZE);
}

void RingLog::InitLogFile_(){
    logfile.seekp(0, std::ios::end);

    // add head
    bzero(&head_, sizeof head_);
    head_.freeList = RINGLOG_BASE_SIZE;
    head_.freeListEnd = Conf::Instance().ring_log_capacity * 1024 - RINGLOG_BASE_SIZE;
    logfile.write((char*)&head_, sizeof head_);

    // add body
    bzero(&body_, sizeof body_);
    int last = Conf::Instance().ring_log_capacity * 1024 / RINGLOG_BASE_SIZE - 1;
    for(int i = 1; i <= last; ++i){
        if(i == 1)
            body_.pre = 0;
        else
            body_.pre = (i - 1) * RINGLOG_BASE_SIZE;
        if(i == last)
            body_.nxt = 0;
        else
            body_.nxt = (i + 1) * RINGLOG_BASE_SIZE;
        logfile.seekp(0, std::ios::end);
        logfile.write((char*)&body_, sizeof body_);
    }
}
void RingLog::RedoTask_(){
    std::unique_lock<std::mutex> locker(mtx_);
    while(head_.todoListNum){
        ReadPage_(&body_, head_.todoList);
        Redo_();
        head_.todoList = body_.nxt;
        head_.todoListNum --;
    }
}
void RingLog::Redo_(){
    if(body_.status & (1ull << RINGLOG_HEAD_ROLLBACK)){
        if(body_.status & (1ull << RINGLOG_HEAD_UPLOAD) == 0){
            if(body_.status & (1ull << RINGLOG_HEAD_USERNAME))
                remove((Conf::Instance().data_path + "/private/" + body_.username + "/" + body_.filename).c_str());
            else
                remove((Conf::Instance().data_path + "/public/" + body_.filename).c_str());
        }
    }
    else if(body_.status & (1ull << RINGLOG_HEAD_SYN)){
        // 查看synlog是否完成
        synLogBody synBody;
        SynLog::Instance().GetPage(&synBody, body_.synLogAddr);
        if(!(synBody.controlHead & (1u << SYNLOG_BASE_STATUS))){
            // 没完成，继续完成
            uint32_t ip = synBody.ip;
            StorageNode *tarPos = nullptr;
            for(auto it: StorageNode::group)
                if(it.second->ip == ip){
                    tarPos = it.second;
                    break;
                }
            if(tarPos == nullptr){
                // LOG
                return;
            }
            tarPos->AddTask(synPack(head_.todoList, body_.synLogAddr, body_.username, body_.filename, 
                body_.status & (1ull << RINGLOG_HEAD_UPLOAD)));
            Epoll::Instance().modFd(tarPos->fd, tarPos, Epoll::connEvent_ | EPOLLOUT);
        }
        else{
            // 已完成，将 ringlog 移到freeList
        }
    }
    
}