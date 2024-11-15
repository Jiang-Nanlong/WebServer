#include "Socket.h"

void Socket::Create() {
    if (sockFd_ > 0) return sockFd_;

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

void Socket::Connect(const InetAddress& addr) {
    if (connect(sockFd_, (struct sockaddr*)&addr.getSockAddr(), sizeof(sockaddr_in)) < 0)
        LOG(ERROR, "socket connect failed");
}

int Socket::Accept(InetAddress& addr) {
    struct sockaddr_in temp;
    socklen_t len;
    memset(&temp, 0, sizeof(sockaddr_in));

    int clnt_sock = accept(sockFd_, (struct sockaddr*)&temp, &len);
    if (clnt_sock < 0) {
        LOG(ERROR, "socket accept failed");
        return -1;
    }
    addr.setSockAddr(temp);
    return clnt_sock;
}

ssize_t Socket::Recv(void* buf, size_t len, int flag) {
    ssize_t len = recv(sockFd_, buf, len, flag);
    if (len <= 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        LOG(ERROR, "socket receive failed");
        return -1;
    }
    return len;
}

ssize_t Socket::NonBlockRecv(void* buf, size_t len) {
    return Recv(buf, len, MSG_DONTWAIT);
}

ssize_t Socket::Send(void* buf, size_t len, int flag) {
    ssize_t len = send(sockFd_, buf, len, flag);
    if (len <= 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        LOG(ERROR, "socket send failed");
        return -1;
    }
    return len;
}

ssize_t Socket::NonBlockSend(void* buf, size_t len) {
    if (len == 0) return 0;
    return Send(buf, len, MSG_DONTWAIT);
}

void Socket::Close() {
    if (sockFd_ != -1) {
        close(sockFd_);
        sockFd_ = -1;
    }
}

void Socket::SetNonBlock() {
    int flag = fcntl(sockFd_, F_GETFL);
    fcntl(sockFd_, F_SETFL, flag | O_NONBLOCK);
}

void Socket::ShutdownWrite() {
    if (shutdown(sockFd_, SHUT_WR) < 0)
        LOG(ERROR, "shutdown write failed");
}

void Socket::SetTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::SetReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::SetReusePort(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::SetKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

