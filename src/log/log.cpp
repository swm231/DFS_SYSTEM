#include "log.h"

Log::Log(): lineCount_(0), isAsync_(false), writeThread_(nullptr), toDay_(0), fp_(nullptr), isOpen_(false){}
Log::~Log(){
    if(writeThread_ && writeThread_->joinable()){
        while(!deque_->empty())
            deque_->consume();
        deque_->Close();
        writeThread_->join();
    }
    if(fp_){
        std::lock_guard<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

void Log::Init(int level = 1, const char *path, const char *suffix, int maxQueueCapacity){
    isOpen_ = true;
    level_ = level;
    if(maxQueueCapacity > 0){
        isAsync_ = true;
        if(!deque_){
            deque_ = std::make_unique<BlockDeque<std::string> >();

            writeThread_ = std::make_unique<std::thread>(&Log::StartLogThread, this);
        }
    }
    else
        isAsync_ = false;

    lineCount_ = 0;
    path_ = path;
    suffix_ = suffix;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d-0%s",
        path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.AddHandledAll();
        if(fp_){
            flush();
            fclose(fp_);
        }

        fp_ = fopen(fileName, "a");
        if(!fp_){
            mkdir(path_, 777);
            fp_ = fopen(fileName, "a");
        }
    }
}

void Log::StartLogThread(){
    Log::Instance().AsyncWrite();
}

void Log::AsyncWrite(){
    std::string str = "";
    while(deque_->pop(str)){
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

void Log::write(int level, const char *format, ...){
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    if(toDay_ != t.tm_mday || (lineCount_ > MAX_LINES)){
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if(toDay_ != t.tm_mday){
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-0%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, lineCount_ / MAX_LINES, suffix_);

        std::unique_lock<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
    }

    {
        std::unique_lock<std::mutex> locker(mtx_);
        lineCount_ ++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld", 
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        AppendLogLevetitle_(level);

        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.FreeBytes(), format, vaList);
        va_end(vaList);
        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if(isAsync_ && deque_ && !deque_->full())  {
            flush();
            deque_->push_back(buff_.UnhandleToStr());
        }
        else
            fputs(buff_.Peek(), fp_);
        buff_.AddHandledAll();
    }
}

void Log::flush(){
    if(isAsync_)
        deque_->consume();
    fflush(fp_);
}

void Log::AppendLogLevetitle_(int level){
    switch (level)
    {
    case 0:
        buff_.Append("[debg]:", 7);
        break;
    case 1:
        buff_.Append("[info]:", 7);
        break;
    case 2:
        buff_.Append("[warn]:", 7);
        break;
    case 3:
        buff_.Append("[erro]:", 7);
        break;
    default:
        buff_.Append("[info]:", 7);
        break;
    }
}

int Log::GetLevel(){
    return level_;
}
void Log::SetLevle(int level){
    level_ = level;
}
bool Log::IsOpen(){
    return isOpen_;
}