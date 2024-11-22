#include "../include/TcpServer.h"

EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if (loop == nullptr)
        LOG(FATAL, "%s:%s:%d mainloop is null", __FILE__, __FUNCTION__, __LINE__);
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr, const string& name) :
    mainLoop_(CheckLoopNotNull(loop)),
    acceptor_(new Acceptor(loop, addr)),
    name_(name),
    ipPort_(addr.getIpPort()),
    threadPool_(new EventLoopThreadPool(loop, name)),
    connectionCallback_(),
    messageCallback_(),
    writeCompleteCallback_(),
    threadInitCallback_(),
    nextConnId_(1),
    started_(0),
    timerWheel_(new TimerWheel(loop))
{
    acceptor_->setNewConnectionCallback(bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto& connection : connections_) {
        ConnectionPtr conn(connection.second);
        connection.second.reset();   // 反正connection_也是类的私有成员变量，有必要手动调用reset函数？类对象析构的时候不是能自动释放吗？
        conn->getLoop()->runInLoop(bind(&Connection::connectDestroyed, conn));
    }
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
        mainLoop_->runInLoop(bind(&Acceptor::listen, acceptor_.get()));
    }
}

// 根据新链接的sockfd和客户端ip封装一个connection，在acceptor中运行
void TcpServer::newConnection(int sockfd, const InetAddress& remoteaddr) {
    EventLoop* loop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    struct sockaddr_in local;
    int len = sizeof(sockaddr_in);
    memset(&local, 0, len);
    if (getsockname(sockfd, (struct sockaddr*)&local, (socklen_t*)&len) < 0)
        LOG(ERROR, "getsockname failed");
    InetAddress localAddr(local);

    ConnectionPtr conn(new Connection(loop, connName, sockfd, localAddr, remoteaddr));
    connections_[connName] = conn;

    conn->enableTimeoutCheck(timerWheel_.get(), 300);  // 默认五分钟后断开连接
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setConnectionCallback(connectionCallback_);
    conn->setCloseCallback(bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    loop->runInLoop(bind(&Connection::connectEstablished, conn));
}

void TcpServer::removeConnection(const ConnectionPtr& conn) {
    LOG(DEBUG, "TcpServer::removeConnection");
    mainLoop_->runInLoop(bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const ConnectionPtr& conn) {
    LOG(DEBUG, "Tcpserver::removeConnectionInLoop");
    connections_.erase(conn->getName());
    EventLoop* loop = conn->getLoop();
    loop->runInLoop(bind(&Connection::connectDestroyed, conn));
}


