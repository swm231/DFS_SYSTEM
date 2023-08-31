#pragma once

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer{
public:
    Buffer(int initBuffSize = 1024);
    Buffer &operator=(const Buffer &other);
    Buffer(Buffer &&other);
    ~Buffer() = default;

    size_t FreeBytes() const;
    size_t UnHandleBytes() const;
    size_t HandledBytes() const;

    const char *Peek() const;
    const char *BeginWriteConst() const;
    char *BeginWrite();

    void AddHandled(size_t len);
    void AddHandledUntil(const char *end);
    void AddHandledAll();

    void HasWritten(size_t len);
    void EnsureFreeBytes(size_t len);

    void Append(const std::string &str);
    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const Buffer &buff);

    ssize_t ReadFd(int fd, int *Errno);
    ssize_t WriteFd(int fd, int *Errno);

    std::string UnhandleToStr();

private:
    char *BeginPtr_();
    const char *BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> LPos_;
    std::atomic<std::size_t> RPos_;
};