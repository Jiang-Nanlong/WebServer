#include "buffer.h"

void Buffer::MoveReadOffset(uint64_t len)
{
    if (len == 0)
        return;
    assert(len <= ReadAbleSize());
    read_index += len;
}

void Buffer::MoveWriteOffset(uint64_t len)
{
    assert(len <= WriteAbleSize());
    write_index += len;
}

void Buffer::EnsureWriteSpace(uint64_t len)
{
    // 如果末尾空闲空间大小足够，直接返回
    if (WriteAbleSize() >= len)
    {
        return;
    }
    if (len <= WriteAbleSize() + FreeSpaceBeforeRead()) // 如果只考虑末尾空间不够,就再加上前边的空闲空间
    {
        uint64_t len = ReadAbleSize();
        copy(ReadIndex(), ReadIndex() + len, Begin());
        read_index = 0;
        write_index = len;
    }
    else
    {
        // 总体空间不够，则需要扩容，不移动数据，直接给写偏移之后扩容足够空间即可
        buf.resize(write_index + len);
    }
}

void Buffer::Write(const void *data, uint64_t len)
{
    if (len == 0)
        return;
    EnsureWriteSpace(len);
    const char *d = (const char *)data;
    copy(d, d + len, WriteIndex());
}

void Buffer::WriteAndPush(const void *data, uint64_t len)
{
    Write(data, len);
    MoveWriteOffset(len);
}

void Buffer::WriteString(const string &data)
{
    return Write(data.c_str(), data.size());
}

void Buffer::WriteStringAndPush(const string &data)
{
    WriteString(data);
    MoveWriteOffset(data.size());
}

void Buffer::WriteBuffer(Buffer &data)
{
    return Write(data.ReadIndex(), data.ReadAbleSize());
}

void Buffer::WriteBufferAndPush(Buffer &data)
{
    WriteBuffer(data);
    MoveWriteOffset(data.ReadAbleSize());
}

void Buffer::Read(void *buf, uint64_t len)
{
    assert(len <= ReadAbleSize());
    copy(ReadIndex(), ReadIndex() + len, (char *)buf);
}

void Buffer::ReadAndPop(void *buf, uint64_t len)
{
    Read(buf, len);
    MoveReadOffset(len);
}

string Buffer::ReadAsString(uint64_t len)
{
    assert(len <= ReadAbleSize());
    string str;
    str.resize(len);
    Read(&str[0], len);
    return str;
}

string Buffer::ReadAsStringAndPop(uint64_t len)
{
    assert(len <= ReadAbleSize());
    string str = ReadAsString(len);
    MoveReadOffset(len);
    return str;
}

char *Buffer::FindCRLF()
{
    char *res = (char *)memchr(ReadIndex(), '\n', ReadAbleSize());
    return res;
}

string Buffer::GetLine()
{
    char *pos = FindCRLF();
    if (pos != NULL)
    {
        return ReadAsString(pos - ReadIndex() + 1);
    }
    else if (ReadAbleSize() > 0)
    {
        return ReadAsString(ReadAbleSize());
    }
    return "";
}

string Buffer::GetLineAndPop()
{
    string str = GetLine();
    MoveReadOffset(str.size());
    return str;
}

void Buffer::Clear()
{
    // 只需要将偏移量归0即可
    read_index = 0;
    write_index = 0;
}
