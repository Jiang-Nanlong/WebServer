#include "EventLoop.h"

EventLoop::EventLoop() :
    isLooping(false),
    threadId(std::this_thread::get_id()),
    WakeUpFd(CreateEventFd()),
    poller(new Poller()),
    isProcessHandleEvents(false),
    isProcessTasks(false),
    WakeUpFdChannel(new Channel(WakeUpFd))
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

void EventLoop::RunInLoop(const Functor& task) {
    if (IsInLoop()) return task();
    return QueueInLoop(task);
}

void EventLoop::QueueInLoop(const Functor& task) {
    {
        lock_guard<mutex> guard(mtx);
        Tasks.push_back(task);
    }
    WakeUp();
}

void EventLoop::ProcessTasks() {
    isProcessTasks = true;
    vector<Functor> functors;
    {
        lock_guard<mutex> guard(mtx);
        functors.swap(Tasks);
    }
    for (auto functor : functors)
        functor();
    isProcessTasks = false;
}

void EventLoop::loop() {
    isLooping = true;
    while (true) {
        vector<Channel*> ReadyChannels = poller->Poll();
        isProcessHandleEvents = true;
        for (auto& it : ReadyChannels)
            it->HandleEvent();
        isProcessHandleEvents = false;

        ProcessTasks();
    }
    isLooping = false;
}