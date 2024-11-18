#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <stdint.h>

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Callbacks.h"

using namespace std;

class EventLoopThreadPool :noncopyable {
private:
    EventLoop* mainLoop_;   // 主reactor
    string name_;
    int threadNum_;         // 从线程数量
    vector<unique_ptr<EventLoopThread>> threads_;
    vector<EventLoop*> loops_;
public:
    EventLoopThreadPool(EventLoop* mainLoop, const string& name);

    ~EventLoopThreadPool() = default;

    void setThreadNum(int num);

    const string& getName() const { return name_; }

    void start(ThreadInitCallback& cb);

    EventLoop* getNextLoop();

    vector<EventLoop*> getAllLoops();
};