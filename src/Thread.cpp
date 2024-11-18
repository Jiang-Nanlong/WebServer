#include "../include/Thread.h"

atomic<int> Thread::threadNum_(0);

Thread::Thread(const EventCallback& func, const string& name) :
    threadId_(std::thread::id()),
    name_(name),
    func_(func),
    joined_(false),
    started_(false),
    mtx_(),
    cond_() {
    ++threadNum_;
    if (name_.empty())
        name_ = "thread" + threadNum_;
}

Thread::~Thread() {
    if (started_ && !joined_)
        thread_->detach();
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}

// 创建一个新线程执行func_函数
void Thread::start() {
    started_ = true;
    thread_ = shared_ptr<thread>(new thread([&] {
        {
            threadId_ = std::this_thread::get_id();
            cond_.notify_one();
        }
        func_();  // 之后这里边会创建一个eventloop，然后开始调用loop()
        started_ = false;
        }));

    unique_lock<mutex> lock(mtx_);
    cond_.wait(lock, [this] { return threadId_ != std::thread::id(); });
}
