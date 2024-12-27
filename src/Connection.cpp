#include "../include/Connection.h"

Connection::Connection(EventLoop* loop,
    const string& name,
    int sockfd,
    const InetAddress& localaddr,
    const InetAddress& remoteaddr) :
    loop_(loop),
    name_(name),
    socket_(new Socket(sockfd)),
    channel_(new Channel(sockfd, loop_)),
    state_(kConnecting),
    localAddr_(localaddr),
    remoteAddr_(remoteaddr),
    highWaterMark_(64 * 1024 * 1024),  // 64M
    connectionCallback_(),
    writeCompleteCallback_(),
    messageCallback_(),
    closeCallback_(),
    highWaterMarkCallback_(),
    timerWheel_(nullptr),
    timeout_(INT_MAX),
    timeoutCheckEnabled_(false)
{
    channel_->setCloseCallback(bind(&Connection::handleClose, this));
    channel_->setReadCallback(bind(&Connection::handleRead, this));
    channel_->setWriteCallback(bind(&Connection::handleWrite, this));
    channel_->setErrorCallback(bind(&Connection::handleError, this));
    socket_->setKeepAlive(true);
    LOG(INFO, "new connection established");
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

void Connection::send(const string& str) {
    if (state_ == kConnected) {
        loop_->runInLoop(std::bind(&Connection::sendInLoop, this, str.c_str(), str.size()));
    }
}

void Connection::sendInLoop(const void* data, size_t len) {
    if (state_ == kDisconnected) {
        LOG(ERROR, "send failed, connection disconnected");
        return;
    }
    int wrote = 0;
    int remaining = len;
    bool errflag = false;
    if (!channel_->isWriteAble() && outputBuffer_.readAbleSize() == 0) {
        wrote = write(channel_->getFd(), data, len);
        if (wrote >= 0) {
            remaining = len - wrote;
            if (remaining == 0) {
                if (writeCompleteCallback_)
                    loop_->queueInLoop(bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else {
            wrote = 0;
            if (errno == EWOULDBLOCK || errno == EAGAIN) {  // EAGAIN和EWOULDBLOCK是非阻塞正常返回
                LOG(ERROR, "Connection sendInLoop failed");
            }
            else if (errno == EPIPE || errno == ECONNRESET) {
                errflag = true;
            }
        }
    }

    // 没有错误发生并且数据没有一次性发送完
    if (!errflag && remaining) {
        int spacetosend = outputBuffer_.readAbleSize();
        if (spacetosend + remaining >= highWaterMark_ && spacetosend < highWaterMark_)
            if (highWaterMarkCallback_)
                loop_->queueInLoop(bind(highWaterMarkCallback_, shared_from_this(), spacetosend + remaining));

        outputBuffer_.write((void*)((char*)data + wrote), remaining);
        if (!channel_->isWriteAble())  // 注册EPOLLOUT事件，socket下一次可写的时候把outputBuffer_缓冲区中的数据继续发送出去
            channel_->enableWriting();   // 所以，channel只要对EPOLLOUT事件感兴趣，就说明outputBuffer_缓冲区中还有数据没有被发送完
    }

}

// 并不是真正的关闭连接，而是把链接的状态修改为disconnecting，并设置半关闭
void Connection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(bind(&Connection::shutdownInLoop, this));
    }
}

void Connection::shutdownInLoop() {
    if (!channel_->isWriteAble())  // 说明outputBuffer_缓冲区中的数据都已经发送完了
        socket_->shutdownWrite();
}

void Connection::connectEstablished() {
    setState(kConnected);
    // channel_->tie(shared_from_this());  // muduo中有这个但我不太理解有什么用
    channel_->enableReading();

    if (connectionCallback_)
        connectionCallback_(shared_from_this());
}

void Connection::connectDestroyed() {
    LOG(DEBUG, "Connection::connectDestroyed");
    if (state_ == kConnected) {
        LOG(DEBUG, "Connection::connectDestroyed state_ == kConnected");
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void Connection::handleRead() {
    int Error = 0;
    int n = inputBuffer_.readFd(channel_->getFd(), &Error);
    if (n > 0) {
        updatetimeoutTimer();
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0) {
        LOG(DEBUG, "Connection::handleRead n = 0");
        handleClose();
    }
    else {
        errno = Error;
        LOG(ERROR, "Connection handleRead failed");
        handleError();
    }
}

void Connection::handleWrite() {
    if (channel_->isWriteAble()) {  // 如果Channel关心写事件，说明发送缓冲区中有数据没有发送完，就发送这些数据
        int Error = 0;
        int n = outputBuffer_.writeFd(channel_->getFd(), &Error);
        if (n > 0) {
            updatetimeoutTimer();
            if (outputBuffer_.readAbleSize() == 0) {  // 输出缓冲区中的数据已经全部写完
                channel_->disableWriting();
                if (writeCompleteCallback_)
                    loop_->queueInLoop(bind(writeCompleteCallback_, shared_from_this()));
                if (state_ == kDisconnecting)  // 说明调用了shutdown函数，但是在调用shutdowninloop的时候发现outputBuffer_缓冲区中还有数据所以没有立即停止，而是调用handlewrite继续写，写完以后才在这里再次调用shutdowninloop
                    shutdownInLoop();
            }
        }
        else
            LOG(ERROR, "Connection handleWrite failed");
    }
    else
        LOG(ERROR, "Connection handleWrite failed: channel can't write");
}

void Connection::handleClose() {
    LOG(DEBUG, "Connection::handleClose");
    setState(kDisconnected);
    channel_->disableAll();

    if (timeoutCheckEnabled_)
        timerWheel_->cancel(socket_->getFd());

    if (connectionCallback_)
        connectionCallback_(shared_from_this());

    if (closeCallback_)
        closeCallback_(shared_from_this());
}

void Connection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->getFd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    }
    else {
        err = optval;
    }
    LOG(ERROR, "Connection handleError: %d", err);
}

const string& Connection::getName() const {
    return name_;
}

EventLoop* Connection::getLoop() const {
    return loop_;
}

bool Connection::isConnected() {
    return state_ == kConnected;
}

const InetAddress& Connection::getLocalAddress() const {
    return localAddr_;
}

const InetAddress& Connection::getRemoteAddress() const {
    return remoteAddr_;
}

void Connection::enableTimeoutCheck(TimerWheel* wheel, int seconds) {
    timerWheel_ = wheel;
    timeout_ = seconds;
    timeoutCheckEnabled_ = true;
    updatetimeoutTimer();
}

void Connection::disableTimeoutCheck() {
    if (timeoutCheckEnabled_.exchange(false) && timerWheel_) {
        timerWheel_->cancel(socket_->getFd());
    }
}

void Connection::updatetimeoutTimer() {
    if (!timeoutCheckEnabled_ || !timerWheel_) {
        return;
    }

    timerWheel_->addTimer(
        socket_->getFd(),
        timeout_,
        bind(&Connection::handleClose, this)
    );
}