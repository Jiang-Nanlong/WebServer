#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <atomic>
#include <functional>
#include <stdio.h>

#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "Connection.h"
#include "Timestamp.h"
#include "Log.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "TimerWheel.h"

using namespace std;

class TcpServer {

private:
    EventLoop* mainLoop_;  // main loop
    unique_ptr<Acceptor> acceptor_;
    unique_ptr<EventLoopThreadPool> threadPool_;

    ConnectionMap connections_;   // 保存所有链接

    TimerPtr timerWheel_;

    const string ipPort_;  // 服务端ip:port
    const string name_;

    // 这些回调函数都是用户自己设置的，然后依次传入Connection的Channel中，最终在Channel中根据相应的事件进行调用
    ConnectionCallback connectionCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    MessageCallback messageCallback_;
    ThreadInitCallback threadInitCallback_;         // EventLoopThread::threadFunc()中loop线程初始化时的回调

    atomic_int started_;

    int nextConnId_;

    void newConnection(int sockfd, const InetAddress& remoteaddr);

    void removeConnection(const ConnectionPtr& conn);

    void removeConnectionInLoop(const ConnectionPtr& conn);

public:
    TcpServer(EventLoop* loop, const InetAddress& addr, const string& name);

    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback& cb);

    void setConnectionCallback(const ConnectionCallback& cb);

    void setWriteCompleteCallback(const WriteCompleteCallback& cb);

    void setMessageCallback(const MessageCallback& cb);

    void setThreadNum(int threadNum);

    void start();
};