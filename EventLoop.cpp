#include "EventLoop.h"

EventLoop::EventLoop() :
    isLooping(false),
    threadId(std::this_thread::get_id()),
    WakeUpFd(CreateEventFd()),
    poller(new Poller()),
    isProcessHandleEvents(false),
    isProcessTasks(false),
    WakeUpFdChannel(new Channel(this, WakeUpFd))
{
    WakeUpFdChannel->SetEvents(EPOLLIN | EPOLLET);
    WakeUpFdChannel->SetReadCallback(bing(&EventLoop::HandleRead, this));
    poller->UpdateChannel(WakeUpFdChannel, EPOLL_CTL_ADD);
}

void EventLoop::HandleRead() {
    uint64_t val = 1;
    int len = read(WakeUpFd, &val, sizeof(val));
    if (len < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            return;
        }
        exit(1);
    }
}

void EventLoop::WakeUp() {
    uint64_t val = 1;
    int len = write(WakeUpFd, &val, sizeof(val));
    if (len < 0) {
        if (errno == EINTR) {
            return;
        }
        exit(1);
    }
}

void EventLoop::RunInLoop(const TaskFunc& task) {
    if (IsInLoop())
        task();
    else
        QueueInLoop(task);
}

void EventLoop::QueueInLoop(const TaskFunc& task) {
    {
        lock_guard<mutex> guard(mtx);
        pendingFunctors.push_back(task);
    }
    WakeUp();
}

void EventLoop::AddChannel(Channel* ch) {
    poller->UpdateChannel(ch, EPOLL_CTL_ADD);
}

void EventLoop::ModifyChannel(Channel* ch) {
    poller->UpdateChannel(ch, EPOLL_CTL_MOD);
}

void EventLoop::RemoveChannel(Channel* ch) {
    poller->UpdateChannel(ch, EPOLL_CTL_DEL);
}

void EventLoop::dopendingFunctors() {
    isProcessTasks = true;
    vector<Functor> functors;
    {
        lock_guard<mutex> guard(mtx);
        functors.swap(pendingFunctors);
    }
    for (auto functor : functors)
        functor();
    isProcessTasks = false;
}

void EventLoop::loop() {
    isLooping = true;
    LOG(INFO, "EventLoop start");
    while (true) {
        vector<Channel*> ReadyChannels;
        poller->Poll(ReadyChannels);
        isProcessHandleEvents = true;
        for (auto& it : ReadyChannels)
            it->HandleEvent();
        isProcessHandleEvents = false;

        dopendingFunctors();
    }
    isLooping = false;
}