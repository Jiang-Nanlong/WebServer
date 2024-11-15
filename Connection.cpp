#include "Connection.h"

Connection::Connection(EventLoop* loop,
    const string& name,
    int sockfd,
    const InetAddress& localaddr,
    const InetAddress& remoteaddr) :
    loop_(loop),
    name_(name),
    socket_(new Socket(sockfd)),
    channel_(new Channel(sockfd, loop_)),
    reading_(true),
    state_(kConnecting),
    localAddr_(localaddr),
    remoteAddr_(remoteaddr),
    highWaterMark_(64 * 1024 * 1024)  // 64M
{
    channel_->setCloseCallback(bind(&Connection::handleClose, this));
    channel_->setReadCallback(bind(&Connection::handleRead, this));
    channel_->setWriteCallback(bind(&Connection::handleWrite, this));
    channel_->setErrorCallback(bind(&Connection::handleError, this));
    socket_->setKeepAlive(true);
}

void Connection::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
}

void Connection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
}

void Connection::setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
}

void Connection::setCloseCallback(const CloseCallback& cb) {
    closeCallback_ = cb;
}

void Connection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb) {
    highWaterMarkCallback_ = cb;
}

void Connection::setState(State s) {
    state_ = s;
}

void Connection::handleRead() {
    int Error = 0;
    int n = inputBuffer_.readFd(channel_->getFd(), &Error);
    if (n > 0)
        messageCallback_(shared_from_this(), &inputBuffer_);  // shared_from_this()会延长对象的生存周期相比于this
    else if (n == 0)
        handleClose();
    else {
        errno = Error;
        LOG(ERROR, "Connection handleRead failed");
        handleError();
    }
}

void Connection::handleWrite() {
    if (channel_->isWriteAble()) {
        int Error = 0;
        int n = outputBuffer_.writeFd(channel_->getFd(), &Error);  // 感觉这里的逻辑有点问题，handleWrite被注册到channel_的写回调了，但是当channel_调用这个函数的时候，又会向channel_绑定的文件描述符写数据
        if (n > 0) {
            if (outputBuffer_.readAbleSize() == 0) {
                channel_->disableAll();
                if (writeCompleteCallback_)
                    loop_->queueInLoop(bind(&Connection::writeCompleteCallback_, shared_from_this()));
            }
            if (state_ == kConnecting)
                shutdownInLoop();
        }
        else
            LOG(ERROR, "Connection handleWrite failed");
    }
    else
        LOG(ERROR, "Connection handleWrite failed: channel can't write");
}

void Connection::handleClose() {
    setState(kDisconnected);
    channel_->disableAll();

    ConnectionPtr connPtr(shared_from_this());
    if (connectionCallback_)
        connectionCallback_(connPtr);

    if (closeCallback_)
        closeCallback_(connPtr);
}

void Connection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    }
    else {
        err = optval;
    }
    LOG(ERROR, "Connection handleError: ", err);
}
