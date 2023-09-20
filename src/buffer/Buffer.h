#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <atomic>
#include <assert.h>
#include <string>
#include <unistd.h>
#include <sys/uio.h> // iovec
#include <cstring>

#define BUFFER_INIT_DEFAULT_SIZE 1024
#define BUFFER_MAX_SIZE 65535

class Buffer
{
private:
    // 字符缓冲区
    std::vector<char> m_buffer;
    // 缓冲区读起始位置
    std::atomic<std::size_t> m_readPos;
    // 缓冲区写起始位置
    std::atomic<std::size_t> m_writePos;

    void makeSpace(size_t len);
    
public:
    Buffer(int nInitSize = BUFFER_INIT_DEFAULT_SIZE);
    ~Buffer() = default;

    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    // 缓冲区头指针
    char* beginPtr();
    const char* beginPtr() const;

    // 缓冲区写指针
    char* writePtr();
    const char* writePtrConst() const;

    // 缓冲区读指针
    char* readPtr();
    const char* readPtrConst() const;

    void retrive(size_t len);
    void retrive(const char* end);
    std::string retriveToStr();
    void clear();

    void hasWritten(size_t len);
    void append(const std::string& str);
    void append(const void* data, size_t len);
    void append(const char* str, size_t len);
    void append(const Buffer& buffer);

    void ensureWritable(size_t len);

    ssize_t readFd(int fd, int *saveError);
    ssize_t writeFd(int fd, int *saveError);

};


#endif