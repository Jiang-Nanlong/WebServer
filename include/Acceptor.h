#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <functional>

#include "Log.h"
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Callbacks.h"

using namespace std;

class EventLoop;

class Acceptor :noncopyable {
private:
    EventLoop* loop_;  // main loop
    Socket acceptSocket_;   // 主线程上用于监听客户端连接的socket，这个socket要设置成非阻塞的
    Channel acceptChannel_;

    NewConnectionCallback newConnectionCallback_;  // tcpserver中设定，因为唯一一个acceptor归tcpserver管理，分发新建立的连接到subreactor

    int createNonblockSocket();

    void handleRead();
public:
    Acceptor(EventLoop* loop, const InetAddress& addr);

    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb);

    void listen();
};