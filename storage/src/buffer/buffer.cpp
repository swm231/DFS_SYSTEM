#include "buffer.h"

Buffer::Buffer(int initBufferSize) : buffer_(initBufferSize), LPos_(0), RPos_(0) {}
Buffer &Buffer::operator=(const Buffer &other){
    return *this;
}
Buffer::Buffer(Buffer &&other): LPos_(other.LPos_.load()), RPos_(other.RPos_.load()){
    buffer_ = std::move(other.buffer_);
}

// 返回还可写的空间长度
size_t Buffer::FreeBytes() const{
    return buffer_.size() - RPos_;
}
// 返回还有多少长度处理
size_t Buffer::UnHandleBytes() const{
    return RPos_ - LPos_;
}
// 返回已读的长度
size_t Buffer::HandledBytes() const{
    return LPos_;
}

// 返回该读的内容
const char* Buffer::Peek() const{
    return BeginPtr_() + LPos_;
}
// 返回该写的地址
const char* Buffer::BeginWriteConst() const{
    return BeginPtr_() + RPos_;
}
// 返回该写的地址
char* Buffer::BeginWrite(){
    return BeginPtr_() + RPos_;
}

void Buffer::AddHandled(size_t len){
    assert(len <= UnHandleBytes());
    LPos_ += len;
}
void Buffer::AddHandledUntil(const char* end){
    assert(Peek() <= end);
    AddHandled(end - Peek());
}
void Buffer::AddHandledAll(){
    bzero(&buffer_[0], buffer_.size());
    LPos_ = 0;
    RPos_ = 0;
}

void Buffer::HasWritten(size_t len){
    RPos_ += len;
}
void Buffer::EnsureFreeBytes(size_t len){
    if(FreeBytes() < len){
        MakeSpace_(len);
    }
    assert(FreeBytes() >= len);
}

void Buffer::Append(const std::string& str){
    Append(str.data(), str.length());
}
void Buffer::Append(const void* data, size_t len){
    assert(data);
    Append(static_cast<const char *>(data), len);
}
void Buffer::Append(const Buffer& buff){
    Append(buff.Peek(), buff.UnHandleBytes());
}
void Buffer::Append(const char* str, size_t len){
    assert(str);
    EnsureFreeBytes(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

ssize_t Buffer::ReadFd(int fd, int* saveErrno){
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = FreeBytes();
    iov[0].iov_base = BeginPtr_() + RPos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof buff;

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0)
        *saveErrno = errno;
    else if(static_cast<size_t>(len) <= writable)
        RPos_ += len;
    else{
        RPos_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}
ssize_t Buffer::WriteFd(int fd, int* saveErrno){
    size_t readSize = UnHandleBytes();
    ssize_t len = write(fd, Peek(), readSize);
    if(len <= 0){
        *saveErrno = errno;
        return len;
    }
    LPos_ += len;
    return len;
}

std::string Buffer::UnhandleToStr(){
    std::string str(Peek(), UnHandleBytes());
    AddHandledAll();
    return str;
}

char* Buffer::BeginPtr_(){
    return &*buffer_.begin();
}
const char* Buffer::BeginPtr_() const{
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len){
    if(FreeBytes() + HandledBytes() < len)
        buffer_.resize(RPos_ + len + 1);
    else{
        size_t readable = UnHandleBytes();
        std::copy(BeginPtr_() + LPos_, BeginPtr_() + RPos_, BeginPtr_());
        LPos_ = 0;
        RPos_ = LPos_ + readable;
        assert(readable == UnHandleBytes());
    }
}