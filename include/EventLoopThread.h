#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>

#include "Thread.h"
#include "EventLoop.h"
#include "noncopyable.h"
#include "Callbacks.h"

using namespace std;

class EventLoopThread :noncopyable {
private:
    EventLoop* loop_;  // thread_中创建的eventloop
    Thread thread_;
    mutex mtx_;
    condition_variable cond_;
    ThreadInitCallback callback_;   // 可以在eventloop对象在开始loop之前执行某些操作

    void threadFunc();

public:
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const string& name = string());

    ~EventLoopThread();

    EventLoop* startLoop();
};