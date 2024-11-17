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

using namespace std;

// 把之前所有的结构部件整合起来
class TcpServer {
    using ConnectionPtr = shared_ptr<Connection>;
    using ConnectionMap = unordered_map<string, ConnectionPtr>;
    using ConnectionCallback = function<void(const ConnectionPtr&)>;
    using CloseCallback = function<void(const ConnectionPtr&)>;
    using WriteCompleteCallback = function<void(const ConnectionPtr&)>;
    using MessageCallback = function<void(const ConnectionPtr&, Buffer*, Timestamp)>;
    using ThreadInitCallback = function<void(EventLoop*)>;
private:
    EventLoop* mainLoop_;  // main loop
    unique_ptr<Acceptor> acceptor_;
    unique_ptr<EventLoopThreadPool> threadPool_;

    ConnectionMap connections_;   // 保存所有链接

    const string ipPort_;  // ip:port
    const string name_;

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