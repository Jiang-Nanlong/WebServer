#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <sys/eventfd.h>

#include "Log.h"
#include "noncopyable.h"
#include "Poller.h"
#include "Channel.h"
#include "Callbacks.h"

using namespace std;

// Reactor
class EventLoop :noncopyable {
private:
    const std::thread::id threadId_;   // 记录创建EventLoop对象的线程id，Channel上发生的事件只能在自己的eventloop线程中处理
    unique_ptr<Poller> poller_;

    int wakeupFd_;
    unique_ptr<Channel> wakeupChannel_;

    vector<Functor> pendingFunctors_;
    mutex mtx_;

    bool isLooping_;
    bool isProcessHandleEvents_;  // 是否正在处理poller返回的vector<Channel*>
    bool isProcessPendingFunctors_;         // 是否正在处理额外任务
    bool isQuit_;

    static int createWakeupFd();

    void dopendingFunctors();
public:
    EventLoop();

    ~EventLoop();

    uint32_t getChannelNum() const;

    void handleRead();

    void wakeup();

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

    void quit();

    void runInLoop(Functor& task);

    void queueInLoop(Functor& task);

    void updateChannel(Channel* ch);

    void removeChannel(Channel* ch);

    void loop();
};