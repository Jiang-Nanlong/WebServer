#include "EventLoopThread.h"


EventLoopThread::EventLoopThread(ThreadInitCallback& cb, const string& name) :
    loop_(nullptr),
    thread_(bind(&EventLoopThread::threadFunc, this), name),
    mtx_(),
    cond_(),
    callback_(cb) {
}

EventLoopThread::~EventLoopThread() {
    if (loop_) {
        loop_->quit();
        thread_.join();
    }
}

// �����´�����eventloop�����ָ��
EventLoop* EventLoopThread::startLoop() {
    thread_.start();  // ͨ��Thread����������һ��eventloop����Ȼ��ʼloop

    {
        unique_lock<mutex> lock(mtx_);
        cond_.wait(lock, [] {return loop_ != nullptr;});
    }
    return loop_;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (callback_)
        callback_();

    {
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();   // �ڴ˴�����eventloop.loop()

    lock_guard<mutex> lock(mtx_);
    loop_ = nullptr;
}