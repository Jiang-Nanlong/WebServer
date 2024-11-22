#pragma once

#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <unordered_map>
#include <sys/timerfd.h>
#include <mutex>

#include "Log.h"
#include "Callbacks.h"
#include "Channel.h"
#include "TimerTask.h"

using namespace std;

class EventLoop;

class TimerWheel {
private:
    EventLoop* loop_;  // main loop
    int timerFd_;
    Channel timerChannel_;

    const int lowCapacity_;
    const int midCapacity_;
    const int highCapacity_;

    int lowTick_;
    int midTick_;
    int highTick_;

    vector<list<TaskPtr>> lowWheel_;
    vector<list<TaskPtr>> midWheel_;
    vector<list<TaskPtr>> highWheel_;

    mutex mtx;
    unordered_map<uint64_t, TaskWeak> timerTasks_;

    static int createTimerFd(int val_sec = 1, int interval_sec = 1);

    void readTimerFd();

    void handleRead();

    void advanceLowWheel();

    void advanceMidWheel();

    void advanceHighWheel();

    void addTimerToLowWheel(uint64_t id, uint32_t timeout, const TaskFunc& cb);

    void addTimerToMidWheel(uint64_t id, uint32_t timeout, const TaskFunc& cb);

    void addTimerToHighWheel(uint64_t id, uint32_t timeout, const TaskFunc& cb);

    void RemoveTimer(uint64_t id);

    TaskPtr createTask(uint64_t id, uint32_t timeout, const TaskFunc& cb);
public:
    TimerWheel(EventLoop* loop, int hourcapacity = 24, int minutecapacity = 60, int secondcapacity = 60, int val_sec = 1, int interval_sec = 1);

    ~TimerWheel();

    void addTimer(uint64_t id, uint32_t timeout, const TaskFunc& cb);

    void addTimerInLoop(uint64_t id, uint32_t timeout, const TaskFunc& cb);

    void cancel(uint64_t id);

    void cancelInLoop(uint64_t id);

    uint32_t getCurrentTime();

};
