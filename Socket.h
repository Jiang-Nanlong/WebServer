#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "Log.h"
#include "noncopyable.h"
#include "InetAddress.h"

using namespace std;

#define LISTEN_NUM 1024

class Socket :noncopyable {
private:
    int sockFd_;
public:
    Socket() :sockFd_(-1) {}

    Socket(int fd) :sockFd_(fd) {}

    ~Socket() { Close(); };

    int getFd() const { return sockFd_; }

    void Create();

    void Bind(const InetAddress& addr);

    void Listen(int num = LISTEN_NUM);

    void Connect(const InetAddress& addr);

    int Accept(InetAddress& addr);  // 把客户端信息通过参数向上层返回

    ssize_t Recv(void* buf, size_t len, int flag = 0);

    ssize_t NonBlockRecv(void* buf, size_t len);

    ssize_t Send(void* buf, size_t len, int flag = 0);

    ssize_t NonBlockSend(void* buf, size_t len);

    void Close();

    void setNonBlock();

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);
};
