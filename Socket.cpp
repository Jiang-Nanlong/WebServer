#include "Socket.h"

bool Socket::Create() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // 此处应该有日志
        return false;
    }
    return true;
}

bool Socket::Bind(const string& ip, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        // 日志
        Close();
        return false;
    }
    return true;
}

bool Socket::Listen(int num) {
    if (listen(sockfd, num) < 0) {
        return false;
    }
    return true;
}

bool Socket::Connect(const string& ip, uint16_t port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return false;
    }
    return true;
}

int Socket::Accept() {
    int clnt_sock = accept(sockfd, nullptr, nullptr);
    if (clnt_sock < 0)
        return -1;
    return clnt_sock;
}

ssize_t Socket::Recv(void* buf, size_t len, int flag) {
    ssize_t str_len = recv(sockfd, buf, len, flag);
    if (str_len <= 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        return -1;
    }
    return str_len;
}

ssize_t Socket::NonBlockRecv(void* buf, size_t len) {
    return Recv(buf, len, MSG_DONTWAIT);
}

ssize_t Socket::Send(void* buf, size_t len, int flag) {
    ssize_t str_len = send(sockfd, buf, len, flag);
    if (str_len <= 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;
        return -1;
    }
    return str_len;
}

ssize_t Socket::NonBlockSend(void* buf, size_t len) {
    if (len == 0) return 0;
    return Send(buf, len, MSG_DONTWAIT);
}

void Socket::Close() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

void Socket::ReuseAddress() {
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));  // 关闭time-wait
    opt = 1;    //SO_REUSEPORT ：设置端口号重用
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (void*)&opt, sizeof(int));
}

void Socket::SetNonBlock() {
    int flag = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);
}

bool Socket::CreateServer(const string& ip, uint16_t port, bool block_flag) {
    if (Create() == false) return false;
    if (block_flag) SetNonBlock();
    if (Bind(ip, port) == false) return false;
    if (Listen() == false) return false;
    ReuseAddress();
    return true;
}

bool Socket::CreateClient(const string& ip, uint16_t port) {
    if (Create() == false) return false;
    if (Connect(ip, port) == false) return false;
    return true;
}
