#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <cstring>
using namespace std;

#define BUFFER_DEFAULT_SIZE 1024


// 用户态缓冲区,有一个读指针和写指针,这两个指针之间夹的是可以读写的数据,其余空间为空闲空间.写指针指向下一个可写的位置

class Buffer
{
private:
    vector<char> buf;
    uint64_t read_index;
    uint64_t write_index;

public:
    Buffer() : read_index(0), write_index(0), buf(BUFFER_DEFAULT_SIZE) {}

    char *Begin() { return &*buf.begin(); }

    char *WriteIndex() { return Begin() + write_index; }

    char *ReadIndex() { return Begin() + read_index; }

    // 计算剩余空间
    uint64_t WriteAbleSize() { return buf.size() - write_index; }

    // 获取缓冲区起始空闲空间大小--读偏移之前的空闲空间
    uint64_t FreeSpaceBeforeRead() { return read_index; }

    // 计算可读数据
    uint64_t ReadAbleSize() { return write_index - read_index; }
    
    // 将读偏移向后移动
    void MoveReadOffset(uint64_t len);
    
    // 将写偏移向后移动
    void MoveWriteOffset(uint64_t len);

    // 确保可写空间足够（整体空闲空间够了就移动数据，否则就扩容）
    void EnsureWriteSpace(uint64_t len);
    
    // 写入数据
    void Write(const void *data, uint64_t len);
    
    void WriteAndPush(const void *data, uint64_t len);
    
    void WriteString(const string &data);
   
    void WriteStringAndPush(const string &data);

    void WriteBuffer(Buffer &data);

    void WriteBufferAndPush(Buffer &data);

    // 读取数据
    void Read(void *buf, uint64_t len);

    void ReadAndPop(void *buf, uint64_t len);

    string ReadAsString(uint64_t len);

    string ReadAsStringAndPop(uint64_t len);

    char *FindCRLF();

    string GetLine();

    string GetLineAndPop();

    void Clear();
};