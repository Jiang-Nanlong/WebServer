#include "../include/EventLoopThread.h"
#include "EventLoopThread.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const string& name) :
    loop_(nullptr),
    thread_(bind(&EventLoopThread::threadFunc, this), name),
    mtx_(),
    cond_(),
    callback_(cb)
{
}

EventLoopThread::~EventLoopThread() {


    if (loop_) {
        loop_->quit();
        thread_.join();
    }
}

// 返回新创建的eventloop对象的指针
EventLoop* EventLoopThread::startLoop() {
    thread_.start();  // 通过Thread对象来创建一个eventloop对象，然后开始loop

    {
        unique_lock<mutex> lock(mtx_);
        cond_.wait(lock, [&] {return loop_ != nullptr;});
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;   // 这个eventloop对象在栈上，运行结束会自动释放，不用担心释放的问题
    if (callback_)
        callback_(&loop);

    {
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();   // 在此处运行eventloop.loop()

    lock_guard<mutex> lock(mtx_);
    loop_ = nullptr;
}