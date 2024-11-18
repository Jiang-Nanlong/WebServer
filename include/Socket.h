#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>

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

    const int getFd() const { return sockFd_; }

    void Create();

    void Bind(const InetAddress& addr);

    void Listen(int num = LISTEN_NUM);

    int Accept(InetAddress& addr);  // 把客户端信息通过参数向上层返回

    void Close();

    void setNonBlock();

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);
};
