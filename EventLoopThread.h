#include "Thread.h"
#include "EventLoop.h"
#include "noncopyable.h"
#include <functional>
#include <mutex>
#include <condition_variable>
using namespace std;

// EventLoopThread类中通过一个Thread对象来管理一个线程，该线程中运行EventLoop.loop()
class EventLoopThread :noncopyable {
private:
    EventLoop* loop_;
    Thread thread_;
    mutex mtx_;
    condition_variable cond_;
    using ThreadInitCallback = function<void(EventLoop*)>;
    ThreadInitCallback callback_;

    void threadFunc();

public:
    EventLoopThread(ThreadInitCallback& cb = ThreadInitCallback(), const string& name = string());

    ~EventLoopThread();

    EventLoop* startLoop();
}