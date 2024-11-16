#pragma once

#include <functional>
#include <unistd.h>

#include "Log.h"
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"

using namespace std;

class EventLoop;

// 运行在mainloop，主要用于监听新的客户端连接
class Acceptor :noncopyable {
    using NewConnectionCallback = function<void(int sockfd, const InetAddress& addr)>;
private:
    EventLoop* loop_;  // main loop
    Socket acceptSocket_;   // 主线程上用于监听客户端连接的socket，这个socket要设置成非阻塞的
    Channel acceptChannel_;

    NewConnectionCallback newConnectionCallback_;  // tcpserver中设定，分发新建立的连接到subreactor

    int createNonblockSocket();

    void handleRead();
public:
    Acceptor(EventLoop* loop, const InetAddress& addr);

    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb);

    void listen();
};