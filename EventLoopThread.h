#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>

#include "Thread.h"
#include "EventLoop.h"
#include "noncopyable.h"

using namespace std;

// EventLoopThread类中通过一个Thread对象来管理一个线程，该线程中运行EventLoop.loop()
class EventLoopThread :noncopyable {
private:
    EventLoop* loop_;  // thread_中创建的eventloop
    Thread thread_;
    mutex mtx_;
    condition_variable cond_;
    using ThreadInitCallback = function<void(EventLoop*)>;
    ThreadInitCallback callback_;   // 可以在eventloop对象在开始loop之前执行某些操作

    void threadFunc();

public:
    EventLoopThread(ThreadInitCallback& cb = ThreadInitCallback(), const string& name = string());

    ~EventLoopThread();

    EventLoop* startLoop();
};