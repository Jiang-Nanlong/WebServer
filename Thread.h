#pragma once

#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <condition_variable>
#include "noncopyable.h"

using namespace std;

// һ��Thread������������һ���̣߳�����߳�������һ��eventloop
class Thread :noncopyable {
private:
    std::thread::id threadId_;   // ������̶߳�Ӧ��id��Ҳ����eventloop�����ж�Ӧ��id
    shared_ptr<thread> thread_;
    using ThreadFunc = function<void()>;
    ThreadFunc func_;
    string name_;
    static atomic_int threadNum_;
    bool joined_;
    bool started_;
    mutex mtx_;
    condition_variable cond_;

public:
    Thread(ThreadFunc& func, const string& name = string());

    ~Thread();

    const thread::id& getThreadId() { return threadId_; }

    const string& getThreadName() { return name_; }

    bool isJoin() { return joined_; }

    void join();

    bool isStart() { return started_; }

    void start();
};