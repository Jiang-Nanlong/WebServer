#pragma once

#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <cstring>
#include <stdint.h>
#include <sys/uio.h>
using namespace std;

#define BUFFER_DEFAULT_SIZE 1024

// 用户态缓冲区
class Buffer {
private:
    vector<char> buf_;
    uint64_t readIndex_;
    uint64_t writeIndex_;

public:
    Buffer(uint64_t buffersize = BUFFER_DEFAULT_SIZE) : readIndex_(0), writeIndex_(0), buf_(buffersize) {}

    ~Buffer() = default;

    char* begin() { return &*buf_.begin(); }

    char* writeIndex() { return begin() + writeIndex_; }

    char* readIndex() { return begin() + readIndex_; }

    uint64_t writeAbleSize() { return buf_.size() - writeIndex_; }

    uint64_t prependAbleSize() { return readIndex_; }

    uint64_t readAbleSize() { return writeIndex_ - readIndex_; }

    void moveReadOffset(uint64_t len);

    void moveWriteOffset(uint64_t len);

    void ensureWriteSpace(uint64_t len);

    void write(const void* data, uint64_t len);

    void writeString(const string& data);

    void writeBuffer(Buffer& data);

    void read(void* buf_, uint64_t len);

    void readAndPop(void* buf_, uint64_t len);

    string readAsString(uint64_t len);

    string readAsStringAndPop(uint64_t len);

    char* findCRLF();

    string getLine();

    string getLineAndPop();

    ssize_t readFd(int fd, int* errorNum);  // 使用errorNum返回错误信息

    ssize_t writeFd(int fd, int* errorNum);

    void clear();
};