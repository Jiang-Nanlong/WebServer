#pragma once

#include <string>
#include <memory>
#include <atomic>

#include "noncopyable.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Buffer.h"

using namespace std;

// 封装一个已建立的连接，TcpConnection对象是在subreactor中管理
class Connection :noncopyable {
private:
    EventLoop* loop_; // 新链接被挂载的sub loop
    string name_;
    enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };
    atomic_int state_;
    bool reading_;

    unique_ptr<Socket> socket_;
    unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress remoteAddr_;

    using ConnectionPtr = shared_ptr<Connection>;
    using ConnectionCallback = function<void(const ConnectionPtr&)>;
    using CloseCallback = function<void(const ConnectionPtr&)>;
    using WriteCompleteCallback = function<void(const ConnectionPtr&)>;
    using MessageCallback = function<void(const ConnectionPtr&, Buffer*, Timestamp)>;
    using HighWaterMarkCallback = function < void(const ConnectionPtr&, size_t)>;

    ConnectionCallback connectionCallback_;         // 有新连接时的回调
    WriteCompleteCallback writeCompleteCallback_;   // 消息发送完以后的回调
    MessageCallback messageCallback_;               // 有读写消息时的回调
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
public:
    Connection(EventLoop* loop, const string& name, int sockfd, const InetAddress& localaddr, const InetAddress& remoteaddr);

    ~Connection() = default;
}