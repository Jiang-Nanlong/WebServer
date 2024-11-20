#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <sys/eventfd.h>
#include <atomic>

#include "Log.h"
#include "noncopyable.h"
#include "Poller.h"
#include "Channel.h"
#include "Callbacks.h"

using namespace std;

class Poller;

// Reactor
class EventLoop :noncopyable {
private:
    const std::thread::id threadId_;   // 记录创建EventLoop对象的线程id，Channel上发生的事件只能在自己的eventloop线程中处理
    unique_ptr<Poller> poller_;
    vector<Channel*> ReadyChannels;

    int wakeupFd_;
    unique_ptr<Channel> wakeupChannel_;

    vector<Functor> pendingFunctors_;
    mutex mtx_;

    atomic<bool> isLooping_;
    atomic<bool> isProcessHandleEvents_;  // 是否正在处理poller返回的vector<Channel*>
    atomic<bool> isProcessPendingFunctors_;         // 是否正在处理额外任务
    atomic<bool> isQuit_;

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

    void runInLoop(Functor task);  // 开始的时候把runInLoop和queueInLoop的参数都设置为了引用传值，但是后边使用bind传参的时候一直报错，后来才知道bind返回的是一个临时对象，而临时对象不能用来引用传参

    void queueInLoop(Functor task);

    void updateChannel(Channel* ch);

    void removeChannel(Channel* ch);

    void loop();
};