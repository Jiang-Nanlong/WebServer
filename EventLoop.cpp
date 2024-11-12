#include "EventLoop.h"

__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;   // epoll_wait超时时间

EventLoop::EventLoop() :
    isLooping_(false),
    threadId_(std::this_thread::get_id()),
    wakeupFd_(createWakeupFd()),
    poller_(new Poller()),
    isProcessHandleEvents_(false),
    isProcessPendingFunctors_(false),
    isQuit_(false),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
    if (t_loopInThisThread)
        LOG(FATAL, "another eventloop is exist:"t_loopInThisThread);
    else {
        t_loopInThisThread = this;
        wakeupChannel_->SetReadCallback(bind(&EventLoop::handleRead, this));
        wakeupChannel_->enableReading();  // wakeupChannel监听读事件
    }
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->Remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead() {
    uint64_t val = 1;
    int len = read(wakeupFd_, &val, sizeof(val));
    if (len != sizeof(val)) {
        LOG(ERROR, "eventloop wakeup read error");
    }
}

void EventLoop::wakeup() {
    uint64_t val = 1;
    int len = write(wakeupFd_, &val, sizeof(val));
    if (len != sizeof(val)) {
        LOG(ERROR, "eventloop wakeup write error");
    }
}

void EventLoop::quit() {
    isQuit_ = true;

    if (!isInLoopThread())  // 在其他线程中可以终止当前线程
        wakeup();
}

void EventLoop::runInLoop(TaskFunc& task) {
    if (isInLoopThread())
        task();
    else
        queueInLoop(task);
}

void EventLoop::queueInLoop(TaskFunc& task) {
    {
        lock_guard<mutex> lock(mtx_);
        pendingFunctors_.emplace_back(task);
    }
    if (!isInLoopThread() || isProcessPendingFunctors_)
        // 当前线程非该eventloop对应的线程 或者 当前线程是eventloop线程，但是正在处理pendingFunctors_，而此时要往pendingFunctors_中加新函数，
        // 防止它执行完本次pendingFunctors_，转过头来执行新加的函数时又被阻塞
        wakeup();
}

void EventLoop::updateChannel(Channel* ch) {
    poller_->UpdateChannel(ch);
}

void EventLoop::removeChannel(Channel* ch) {
    poller_->RemoveChannel(ch);
}

int EventLoop::createWakeupFd() {
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0) {
        exit(1);
    }
    return fd;
}

void EventLoop::dopendingFunctors() {
    isProcessPendingFunctors_ = true;
    vector<Functor> functors;
    {
        lock_guard<mutex> lock(mtx_);
        functors.swap(pendingFunctors_);   // 避免在pendingFunctors_中直接执行函数时会长期占有锁，导致其他线程无法及时向pendingFunctors_中添加函数
    }
    for (auto functor : functors)
        functor();
    isProcessPendingFunctors_ = false;
}

void EventLoop::loop() {
    isLooping_ = true;
    isQuit_ = false;
    LOG(INFO, "EventLoop start");
    while (!isQuit_) {
        vector<Channel*> ReadyChannels;
        poller_->Poll(kPollTimeMs, ReadyChannels);
        isProcessHandleEvents_ = true;
        for (auto& it : ReadyChannels)
            it->HandleEvent();
        isProcessHandleEvents_ = false;

        dopendingFunctors();
    }
    isLooping_ = false;
}