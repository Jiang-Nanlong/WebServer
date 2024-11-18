#include "../include/Buffer.h"

Buffer::Buffer(uint64_t buffersize) :
    buf_(buffersize),
    readIndex_(0),
    writeIndex_(0)
{
}

void Buffer::moveReadOffset(uint64_t len) {
    if (len == 0)
        return;
    if (len <= readAbleSize())
        readIndex_ += len;
}

void Buffer::moveWriteOffset(uint64_t len) {
    if (len <= writeAbleSize())
        writeIndex_ += len;
}

void Buffer::ensureWriteSpace(uint64_t len) {
    if (writeAbleSize() >= len) {
        return;
    } if (len <= writeAbleSize() + prependAbleSize()) {

        uint64_t len = readAbleSize();
        copy(readIndex(), readIndex() + len, begin());
        readIndex_ = 0;
        writeIndex_ = len;
    }
    else {
        buf_.resize(writeIndex_ + len);
    }
}

void Buffer::write(const void* data, uint64_t len) {
    if (len == 0)
        return;
    ensureWriteSpace(len);
    const char* d = (const char*)data;
    copy(d, d + len, writeIndex());
    moveWriteOffset(len);
}

void Buffer::writeString(const string& data) {
    return write(data.c_str(), data.size());
}

void Buffer::writeBuffer(Buffer& data) {
    return write(data.readIndex(), data.readAbleSize());
}

void Buffer::read(void* buf_, uint64_t len) {
    if (len <= readAbleSize())
        copy(readIndex(), readIndex() + len, (char*)buf_);
}

void Buffer::readAndPop(void* buf_, uint64_t len) {
    read(buf_, len);
    moveReadOffset(len);
}

string Buffer::readAsString(uint64_t len) {
    if (len > readAbleSize())
        len = readAbleSize();
    string str;
    str.resize(len);
    read(&str[0], len);
    return str;
}

string Buffer::readAsStringAndPop(uint64_t len) {
    if (len > readAbleSize())
        len = readAbleSize();
    string str = readAsString(len);
    moveReadOffset(len);
    return str;
}

char* Buffer::findCRLF() {
    char* res = (char*)memchr(readIndex(), '\n', readAbleSize());
    return res;
}

string Buffer::getLine() {
    char* pos = findCRLF();
    if (pos != NULL) {
        return readAsString(pos - readIndex() + 1);
    }
    else if (readAbleSize() > 0) {
        return readAsString(readAbleSize());
    }
    return "";
}

string Buffer::getLineAndPop() {
    string str = getLine();
    moveReadOffset(str.size());
    return str;
}

// 从socket读数据到缓冲区
ssize_t Buffer::readFd(int fd, int* errorNum) {
    char extrabuf[65536]; //64k
    struct iovec vec[2];
    vec[0].iov_base = writeIndex();
    int writeAbleLen = writeAbleSize();
    vec[0].iov_len = writeAbleLen;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    ssize_t n = readv(fd, vec, 2);
    if (n < 0)
        *errorNum = errno;
    else if (n <= writeAbleLen) {
        writeIndex_ += n;
    }
    else {
        writeIndex_ = buf_.size();
        write((void*)extrabuf, n - writeAbleLen);
    }

    return n;
}

// 把缓冲区中的数据写到socket
ssize_t Buffer::writeFd(int fd, int* errorNum) {
    ssize_t n = ::write(fd, readIndex(), readAbleSize());
    if (n < 0)
        *errorNum = errno;
    else
        moveReadOffset(n);

    return n;
}

void Buffer::clear() {
    readIndex_ = 0;
    writeIndex_ = 0;
}
