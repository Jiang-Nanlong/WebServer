#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* mainLoop, const string& name) :
    mainLoop_(mainLoop),
    name_(name),
    threadNum_(0)
{
}

void EventLoopThreadPool::setThreadNum(int num) {
    threadNum_ = num;
    threads_.resize(num);
    loops_.resize(num);
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    for (int i = 0;i < threadNum_;i++) {
        string name = name_ + to_string(i);
        EventLoopThread* t = new EventLoopThread(cb, name);
        threads_[i] = unique_ptr<EventLoopThread>(t);
        loops_[i] = t->startLoop();
    }

    if (threadNum_ == 0 && cb)
        cb(mainLoop_);
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = mainLoop_;
    if (threadNum_ != 0) {
        uint32_t num = UINT_MAX;
        for (auto& it : loops_) {
            uint32_t listenSocketNum = it->getChannelNum();
            if (num > listenSocketNum) {
                num = listenSocketNum;
                loop = it;
            }
        }
    }
    return loop;
}

vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if (threadNum_ == 0)
        return vector<EventLoop*>(mainLoop_);
    return loops_;
}
