#pragma once

#include <string>
#include <memory>
#include <vector>

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

using namespace std;

class EventLoopThreadPool :noncopyable {
private:
    EventLoop* mainLoop_;   // 主reactor
    string name_;
    int threadNum_;         // 从线程数量
    int next_;
    vector<unique_ptr<EventLoopThread>> threads_;
    vector<EventLoop*> loops_;

    using ThreadInitCallback = function<void(EventLoop*)>;


public:
    EventLoopThreadPool(EventLoop* mainLoop, const string& name);

    ~EventLoopThreadPool() = default;

    void setThreadNum(int num);

    const string& getName() { return name_; }

    void start(ThreadInitCallback& cb);

    EventLoop* getNextLoop();

    vector<EventLoop*> getAllLoops();
};