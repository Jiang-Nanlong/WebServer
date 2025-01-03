#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <condition_variable>

#include "noncopyable.h"
#include "Callbacks.h"
#include "Log.h"

using namespace std;

class Thread :noncopyable {
private:
    std::thread::id threadId_;    // 管理的线程对应的id，也就是eventloop对象中对应的id
    shared_ptr<thread> thread_;   // 感觉这个可以换成unique_ptr
    EventCallback func_;
    string name_;
    static atomic<int> threadNum_;
    bool joined_;
    bool started_;
    mutex mtx_;
    condition_variable cond_;

public:
    Thread(const EventCallback& func, const string& name = string());

    ~Thread();

    const thread::id& getThreadId() const { return threadId_; }

    const string& getThreadName() const { return name_; }

    bool isJoin() const { return joined_; }

    void join();

    bool isStart() const { return started_; }

    void start();
};