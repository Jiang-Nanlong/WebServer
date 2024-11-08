#pragma once

#include <thread>
#include "Poller.h"
#include "Channel.h"
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <sys/eventfd.h>

using namespace std;

class EventLoop {
private:
    const pid_t threadId;   // 记录创建EventLoop对象的线程号
    shared_ptr<Poller> poller;
    int WakeUpFd;    // 
    shared_ptr<Channel> WakeUpFdChannel;
    using Functor = function<void()>;
    vector<Functor> Tasks;
    mutex mtx;

    bool isLooping;
    bool isProcessHandleEvents;  // 是否正在处理poller返回的vector<Channel*>
    bool isProcessTasks;         // 是否正在处理额外任务

    static int CreateWakeUpFd() {
        int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (fd < 0) {
            exit(1);
        }
        return fd;
    }

    void ProcessTasks();
public:
    EventLoop();

    void HandleRead();

    void WakeUp();

    bool isInLoopThread() const { return threadId == std::this_thread::get_id(); }

    void RunInLoop(const TaskFunc& task);

    void QueueInLoop(const TaskFunc& task);

    void loop();
};