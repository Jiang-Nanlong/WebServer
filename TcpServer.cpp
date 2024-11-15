#include "TcpServer.h"

EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if (loop == nullptr)
        LOG(FATAL, "%s:%s:%d mainloop is null", __FILE__, __FUNCTION__, __LINE__);
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr, const string& name) :
    mainLoop_(CheckLoopNotNull(loop)),
    acceptor_(new Acceptor(mainloop_, addr)),
    name_(name),
    ipPort_(addr.toIpPort()),
    threadPool_(new EventLoopThreadPool(mainLoop_, name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback& cb) {
    threadInitCallback_ = cb;
}

void TcpServer::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
}

void TcpServer::setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
}

void TcpServer::setThreadNum(int threadNum) {
    threadPool_->setThreadNum(threadNum);
}

void TcpServer::start() {
    if (started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        mainLoop_->runInLoop(bind(&Acceptor::listen(), acceptor_.get()));
    }
}
