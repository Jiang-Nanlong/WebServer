#include "../include/EventLoop.h"

__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;   // epoll_wait超时时间

EventLoop::EventLoop() :
    threadId_(std::this_thread::get_id()),
    wakeupFd_(createWakeupFd()),
    wakeupChannel_(new Channel(wakeupFd_, this)),
    poller_(new Poller()),
    isLooping_(false),
    isProcessHandleEvents_(false),
    isProcessPendingFunctors_(false),
    isQuit_(false)
{
    if (t_loopInThisThread)
        LOG(FATAL, "another eventloop is exist: %p", t_loopInThisThread);
    else {
        t_loopInThisThread = this;
        wakeupChannel_->setReadCallback(bind(&EventLoop::handleRead, this));
        wakeupChannel_->enableReading();
    }
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

uint32_t EventLoop::getChannelNum() const {
    return poller_->getChannelNum();
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

    if (!isInLoopThread())
        wakeup();
}

void EventLoop::runInLoop(Functor task) {
    if (isInLoopThread())
        task();
    else
        queueInLoop(task);
}

void EventLoop::queueInLoop(Functor task) {
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
    poller_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel* ch) {
    poller_->removeChannel(ch);
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
        ReadyChannels.clear();
        poller_->poll(kPollTimeMs, ReadyChannels);
        isProcessHandleEvents_ = true;
        for (auto& it : ReadyChannels)
            it->handleEvent();
        isProcessHandleEvents_ = false;

        dopendingFunctors();
    }
    isLooping_ = false;
}