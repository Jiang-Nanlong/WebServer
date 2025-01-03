#include "../include/Socket.h"

void Socket::Create() {
    if (sockFd_ > 0) return;

    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd_ < 0) {
        LOG(ERROR, "creat socket failed");
    }
}

void Socket::Bind(const InetAddress& addr) {
    if (bind(sockFd_, (struct sockaddr*)addr.getSockAddr(), sizeof(sockaddr_in)) < 0) {
        LOG(FATAL, "socket bind failed");
        Close();
    }
}

void Socket::Listen(int num) {
    if (listen(sockFd_, num) < 0)
        LOG(ERROR, "socket listen failed");
}

int Socket::Accept(InetAddress& addr) {
    struct sockaddr_in temp;
    socklen_t len = sizeof(sockaddr_in);
    memset(&temp, 0, sizeof(sockaddr_in));

    int clnt_sock = accept4(sockFd_, (struct sockaddr*)&temp, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);  // 新链接的sockfd设为非阻塞
    if (clnt_sock < 0) {
        LOG(ERROR, "socket accept failed");
        return -1;
    }
    addr.setSockAddr(temp);
    return clnt_sock;
}

void Socket::Close() {
    if (sockFd_ != -1) {
        close(sockFd_);
        sockFd_ = -1;
    }
}

void Socket::setNonBlock() {
    int flag = fcntl(sockFd_, F_GETFL);
    fcntl(sockFd_, F_SETFL, flag | O_NONBLOCK);
}

void Socket::shutdownWrite() {
    if (shutdown(sockFd_, SHUT_WR) < 0)
        LOG(ERROR, "shutdown write failed");
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

