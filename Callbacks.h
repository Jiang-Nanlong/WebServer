#pragma once

#include <functional>
#include <memory>
#include <string>

#include "InetAddress.h"

class Channel;
class EventLoop;
class Connection;
class Buffer;

// Channel.h
using EventCallback = function<void()>;

// Poller.h
using ChannelMap = unordered_map<int, Channel*>;
// 这里直接使用Channel*来指向一个Channel，而不是使用智能指针是因为Poller不负责Channel的生命周期的管理，在上层的Connection中负责管理。
// Connection中使用unique_ptr来管理一个Channel对象，而一个Connection对象的生命周期和对应的Channel的生命周期是相同的，当一个Connection断开时，会从底层Poller中移除对Channel的监听，
// 然后Connection对象析构时，智能指针会自动释放它管理的Channel对象。
using EventList = vector<struct epoll_event>;

// EventLoop.h
using Functor = function<void()>;

// EventLoopThread.h   EventLoopThreadPool.h
using ThreadInitCallback = function<void(EventLoop*)>;

// Acceptor.h
using NewConnectionCallback = function<void(int sockfd, const InetAddress& addr)>;

//Connection.h
using ConnectionPtr = shared_ptr<Connection>;
using ConnectionCallback = function<void(const ConnectionPtr&)>;
using CloseCallback = function<void(const ConnectionPtr&)>;
using WriteCompleteCallback = function<void(const ConnectionPtr&)>;
using MessageCallback = function<void(const ConnectionPtr&, Buffer*)>;
using HighWaterMarkCallback = function < void(const ConnectionPtr&, size_t)>;

// TcpServer.h
using ConnectionMap = unordered_map<string, ConnectionPtr>;
