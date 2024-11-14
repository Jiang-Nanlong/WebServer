#include "Acceptor.h"


Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr) :
    loop_(loop),
    acceptSocket_(createNonblockSocket()),
    acceptChannel_(acceptSocket_.getFd(), loop_) {
    acceptSocket_.SetReuseAddr(true);
    acceptSocket_.SetReusePort(true);
    acceptSocket_.Bind(addr);
    acceptChannel_.setReadCallback(bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();

}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
}

void Acceptor::listen() {
    acceptSocket_.Listen();
    acceptChannel_.enableReading();
}

int Acceptor::createNonblockSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
        LOG(FATAL, "main loop listen socket create failed");
    return sockfd;
}

// 当有新客户端申请链接的时候被调用
void Acceptor::handleRead() {
    InetAddress addr;
    int clintfd = acceptSocket_.Accept(addr);
    if (clintfd > 0) {
        if (newConnectionCallback_)  // 如果设置了函数，则分发；否则关闭
            newConnectionCallback_(clintfd, addr);
        else
            close(clintfd);
    }
    else {
        LOG(ERROR, "main reactor listen client failed");
        if (errno == EMFILE)
            LOG(ERROR, "sockfd reach limit");
    }
}
