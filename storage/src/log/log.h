#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <functional>

#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log{
public:
    static Log &Instance(){
        static Log instance;
        return instance;
    }

    void Init(int level, const char *path = "../Log", const char *suffix = ".log", int maxQueueCapacity = 1000);
    void ShutDown();
    void StartLogThread();
    void AsyncWrite();

    void write(int level, const char *format, ...);
    void flush();

    int GetLevel();
    void SetLevle(int level);
    bool IsOpen();

private:
    Log();
    ~Log();

    void AppendLogLevetitle_(int level);

    const char *path_;
    const char *suffix_;

    int MAX_LINES_;

    int lineCount_;
    int toDay_;

    bool isOpen_;

    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE *fp_;
    std::unique_ptr<BlockDeque<std::string> > deque_;
    std::unique_ptr<std::thread> writeThread_;
    std::mutex mtx_;

    static const int MAX_LINES = 500000;
    static const int LOG_NAME_LEN = 256;
};

#define LOG_BASE(level, format, ...) \
    do { \
        if (Log::Instance().IsOpen() && Log::Instance().GetLevel() <= level) { \
            Log::Instance().write(level, format, ##__VA_ARGS__); \
            Log::Instance().flush(); \
        } \
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);