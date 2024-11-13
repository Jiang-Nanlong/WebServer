#include "Thread.h"

atomic_int threadNum_(0);

Thread::Thread(ThreadFunc& func, const string& name) :
    threadId_(0),
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

// ����һ�����߳�ִ��func_����
void Thread::start() {
    started_ = true;
    thread_ = shared_ptr<thread>(new thread([&] {
        {
            threadId_ = std::this_thread::get_id();
            cond_.notify_one();
        }
        func_();  // ֮������߻ᴴ��һ��eventloop��Ȼ��ʼ����loop()
        started_ = false;
        }));

    unique_lock<mutex> lock(mtx_);
    cond_.wait(lock, [&] {return threadId_ != 0;});
}
