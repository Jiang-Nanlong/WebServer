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
#include "Callbacks.h"
#include "TimerWheel.h"

using namespace std;

enum State { kDisconnected, kConnecting, kConnected, kDisconnecting };

// 封装一个已建立的连接，TcpConnection对象是在subreactor中管理
class Connection :noncopyable, public enable_shared_from_this<Connection> {
private:
    EventLoop* loop_; // 新链接被挂载的sub loop
    string name_;
    atomic_int state_;

    unique_ptr<Socket> socket_;
    unique_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress remoteAddr_;

    TimerWheel* timerWheel_;
    int timeout_;
    atomic<bool> timeoutCheckEnabled_;

    // 这些回调会被注册到channel中，在tcpserver中被设置
    ConnectionCallback connectionCallback_;         // 有新连接时的回调
    WriteCompleteCallback writeCompleteCallback_;   // 消息发送完以后的回调
    MessageCallback messageCallback_;               // 有读写消息时的回调
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;

    // 这些handles只完成基本操作，用户定制的操作可由以上的callbacks实现，这些callback在tcpserver中由用户自己设置
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

    bool isConnected();

    const InetAddress& getLocalAddress() const;

    const InetAddress& getRemoteAddress() const;

    void enableTimeoutCheck(TimerWheel* wheel, int seconds) {
        timerWheel_ = wheel;
        timeout_ = seconds;
        timeoutCheckEnabled_ = true;
        updatetimeoutTimer();
    }

    void disableTimeoutCheck() {
        if (timeoutCheckEnabled_.exchange(false) && timerWheel_) {
            timerWheel_->cancel(socket_->getFd());
        }
    }

    void updatetimeoutTimer() {
        if (!timeoutCheckEnabled_ || !timerWheel_) {
            return;
        }

        timerWheel_->addTimer(
            socket_->getFd(),
            timeout_,
            bind(&Connection::handleClose, this)
        );
    }
};