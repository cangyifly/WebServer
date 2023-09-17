#include "Buffer.h"

// 构造函数
Buffer::Buffer(int nInitSize) : m_buffer(nInitSize), m_readPos(0), m_writePos(0) {}

// 缓冲区头指针
char* Buffer::beginPtr()
{
    return &*m_buffer.begin();
}

const char* Buffer::beginPtr() const
{
    return &*m_buffer.begin();
}

// 缓冲区写指针
char* Buffer::writePtr()
{
    return beginPtr() + m_writePos;
}

const char* Buffer::writePtr() const
{
    return beginPtr() + m_writePos;
}

// 缓冲区读指针
char* Buffer::readPtr()
{
    return beginPtr() + m_readPos;
}

const char* Buffer::readPtr() const
{
    return beginPtr() + m_readPos;
}

// 可读长度
size_t Buffer::readableBytes() const
{
    return m_writePos - m_readPos;
}

// 可写长度
size_t Buffer::writableBytes() const
{
    return m_buffer.size() - m_writePos;
}

// 前驱
size_t Buffer::prependableBytes() const
{
    return m_readPos;
}

// 读取len长度的字符
void Buffer::retrive(size_t len)
{
    assert(len <= readableBytes());
    m_readPos += len;
}

// 读取字符串直到end
void Buffer::retrive(const char* end)
{
    assert(readPtr() <= end);
    retrive(end - readPtr());
}

// 清空
void Buffer::clear()
{
    // TODO:
    // bzero(&m_buffer[0]...)
    // bzero(&m_buffer, m_buffer.size());
    std::fill(m_buffer.begin(), m_buffer.end(), '\0');
    m_readPos = 0;
    m_writePos = 0;
}

// 将读缓冲区的内容转为string，然后clear
std::string Buffer::retriveToStr()
{
    // 起始，长度
    std::string str(readPtr(), readableBytes());
    clear();
    return str;
}

// 修改m_writePos
void Buffer::hasWritten(size_t len)
{
    assert(len <= writableBytes());
    m_writePos += len;
}

// 写入string
void Buffer::append(const std::string& str)
{
    // str.ctr() || str.data()
    append(str.c_str(), str.size());
}

// 写入void
void Buffer::append(const void* data, size_t len)
{
    assert(data);
    append(static_cast<const char*>(data), len);
}

// 写入字符数组
void Buffer::append(const char* str, size_t len)
{
    assert(str);
    ensureWritable(len);
    std::copy(str, str + len, writePtr());
    hasWritten(len);
}

// 写入一个Buffer对象
void Buffer::append(const Buffer& buffer)
{
    append(buffer.readPtr(), buffer.readableBytes());
}

void Buffer::ensureWritable(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
    assert(len <= writableBytes());
}

void Buffer::makeSpace(size_t len)
{
    // 如果所有可写的区域都不够，则扩容
    if (writableBytes() + prependableBytes() < len)
    {
        m_buffer.resize(m_writePos + len + 1);
    }
    else
    {
        // 将读区左移到开头
        std::copy(readPtr(), readPtr() + readableBytes(), beginPtr());
        m_readPos = 0;
        m_writePos = m_readPos + readableBytes();
    }
}

// 将读缓冲区的内容写入文件
ssize_t Buffer::writeFd(int fd, int* saveError)
{
    ssize_t len = write(fd, readPtr(), readableBytes());
    if (len < 0)
    {
        *saveError = errno;
        return len;
    }

    m_readPos += len;
    return len;
}

// 将文件里的内容读入写缓冲区
ssize_t Buffer::readFd(int fd, int* saveError)
{
    char buff[BUFFER_MAX_SIZE] = {0};
    struct iovec iov[2];
    const size_t writableSize = writableBytes();
    // 分散读
    iov[0].iov_base = writePtr();
    iov[0].iov_len = writableBytes();
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *saveError = errno;
    }
    // 只读取了第一块
    else if (static_cast<size_t>(len) <= writableSize)
    {
        m_writePos += len;
    }
    // 读取了两块
    else
    {
        m_writePos = m_buffer.size();
        append(buff, len - writableSize);
    }
    return len;
}


