#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <errno.h>

#include "noncopyable.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Buffer.h"

using namespace std;

enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };

// 封装一个已建立的连接，TcpConnection对象是在subreactor中管理
class Connection :noncopyable, public enable_shared_from_this<Connection> {
    using ConnectionPtr = shared_ptr<Connection>;
    using ConnectionCallback = function<void(const ConnectionPtr&)>;
    using CloseCallback = function<void(const ConnectionPtr&)>;
    using WriteCompleteCallback = function<void(const ConnectionPtr&)>;
    using MessageCallback = function<void(const ConnectionPtr&, Buffer*)>;
    using HighWaterMarkCallback = function < void(const ConnectionPtr&, size_t)>;
private:
    EventLoop* loop_; // 新链接被挂载的sub loop
    string name_;
    atomic_int state_;
    bool reading_;

    unique_ptr<Socket> socket_;
    unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress remoteAddr_;

    // 这些回调会被注册到channel中
    ConnectionCallback connectionCallback_;         // 有新连接时的回调
    WriteCompleteCallback writeCompleteCallback_;   // 消息发送完以后的回调
    MessageCallback messageCallback_;               // 有读写消息时的回调
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void shutdownInLoop();

    void setState(State s);

    void sendInLoop(const void* data, size_t len);
public:
    Connection(EventLoop* loop, const string& name, int sockfd, const InetAddress& localaddr, const InetAddress& remoteaddr);

    ~Connection() = default;

    void setConnectionCallback(const ConnectionCallback& cb);

    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    void setMessageCallback(const MessageCallback& cb);

    void setCloseCallback(const CloseCallback& cb);

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb);

    void send(const string& str);

    void shutdown();

    void connectEstablished();

    void connectDestroyed();

    const string& getName() const;

    EventLoop* getLoop() const;

}