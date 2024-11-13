#include "EventLoopThreadPool.h"

EventLoopThreadLoop::EventLoopThreadLoop(EventLoop* mainLoop, const string& name) :
    mainLoop_(mainLoop),
    name_(name),
    threadNum_(0),
    next_(0)
{
}

void EventLoopThreadLoop::setThreadNum(int num) {
    threadNum_ = num;
    threads_.resize(num);
    loops_.resize(num);
}

void EventLoopThreadLoop::start(const ThreadInitCallback& cb) {
    for (int i = 0;i < threadNum_;i++) {
        string name = name_ + to_string(i);
        EventLoopThread* t = new EventLoopThread(cb, name);
        threads_[i] = unique_ptr<EventLoopThread>(t);
        loops_[i] = t->startLoop();
    }

    if (threadNum_ == 0 && cb)
        cb(mainLoop_);
}

EventLoop* EventLoopThreadLoop::getNextLoop() {
    EventLoop* loop = mainLoop_;
    if (threadNum_ == 0) {
        loop = loops_[next_];
        next_ = (next_ + 1) % threadNum_;
    }
    return loop;
}

vector<EventLoop*> EventLoopThreadLoop::getAllLoops()
{
    if (threadNum_ == 0)
        return vector<EventLoop*>(mainLoop_);
    return loops_;
}
