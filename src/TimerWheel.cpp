#include "TimerWheel.h"

TimerWheel::TimerWheel(EventLoop* loop, int hourcapacity, int minutecapacity, int secondcapacity, int val_sec, int interval_sec) :
    loop_(loop),
    timerFd_(createTimerFd(val_sec, interval_sec)),
    timerChannel_(timerFd_, loop),
    lowCapacity_(secondcapacity),
    midCapacity_(minutecapacity),
    highCapacity_(hourcapacity),
    lowTick_(0),
    midTick_(0),
    highTick_(0),
    mtx(),
    timerTasks_()
{
    lowWheel_.resize(lowCapacity_);
    midWheel_.resize(midCapacity_);
    highWheel_.resize(highCapacity_);

    timerChannel_.setReadCallback(bind(&TimerWheel::handleRead, this));
    timerChannel_.enableReading();
}

TimerWheel::~TimerWheel() {
    timerChannel_.disableAll();
    timerChannel_.remove();
    close(timerFd_);

    for (auto& it : timerTasks_) {
        TaskPtr tp = it.second.lock();
        if (tp)
            tp->cancel();
    }
}

void TimerWheel::addTimer(uint64_t id, uint32_t timeout, const TaskFunc& cb) {
    loop_->runInLoop(bind(&TimerWheel::addTimerInLoop, this, id, timeout, cb));
}

// 添加定时任务的时候即有指针移动，又有任务迁移
void TimerWheel::addTimerInLoop(uint64_t id, uint32_t timeout, const TaskFunc& cb) {
    lock_guard<mutex> lock(mtx);
    if (timeout < lowCapacity_) {
        addTimerToLowWheel(id, timeout, cb);
    }
    else if (timeout < lowCapacity_ * midCapacity_) {
        addTimerToMidWheel(id, timeout, cb);
    }
    else {
        addTimerToHighWheel(id, timeout, cb);
    }
}

void TimerWheel::cancel(uint64_t id) {
    loop_->runInLoop(bind(&TimerWheel::cancelInLoop, this, id));
}

void TimerWheel::cancelInLoop(uint64_t id) {
    auto it = timerTasks_.find(id);
    if (it != timerTasks_.end()) {
        TaskPtr tp = it->second.lock();
        if (tp) tp->cancel();
    }
}

uint32_t TimerWheel::getCurrentTime() {
    uint32_t currenttime = lowTick_ + midTick_ * lowCapacity_ + highTick_ * midCapacity_ * lowCapacity_;
    return currenttime;
}


int TimerWheel::createTimerFd(int val_sec, int interval_sec) {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd == -1) {
        LOG(FATAL, "timer create failed");
    }

    struct itimerspec itime;
    itime.it_value.tv_sec = val_sec;
    itime.it_value.tv_nsec = 0;

    itime.it_interval.tv_sec = interval_sec;
    itime.it_interval.tv_nsec = 0;

    int n = timerfd_settime(timerfd, 0, &itime, NULL);
    if (n == -1) {
        LOG(FATAL, "timer set failed");
    }
    return timerfd;
}

void TimerWheel::readTimerFd() {
    uint64_t times;
    int n = read(timerFd_, &times, 8);
    if (n < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return;
        }
        LOG(FATAL, "Timer Expiration");
    }
}

void TimerWheel::handleRead() {
    readTimerFd();
    advanceLowWheel();
}

void TimerWheel::advanceLowWheel() {
    lowTick_ = (lowTick_ + 1) % lowCapacity_;
    if (lowTick_ == 0) {
        advanceMidWheel();
    }

    lowWheel_[lowTick_].clear();
}

void TimerWheel::advanceMidWheel() {
    lock_guard<mutex> lock(mtx);
    midTick_ = (midTick_ + 1) % midCapacity_;
    if (midTick_ == 0) {
        advanceHighWheel();
    }

    uint32_t currenttime = getCurrentTime();
    while (!midWheel_[midTick_].empty()) {
        auto& task = midWheel_[midTick_].front();
        uint32_t elapsedTime = currenttime - task->getStartTime();
        uint32_t remainingTime = task->getTimeout() - elapsedTime;

        int newPos = (remainingTime + lowTick_) % lowCapacity_;
        lowWheel_[newPos].push_back(task);
        midWheel_[midTick_].pop_front();
    }
}

void TimerWheel::advanceHighWheel() {
    highTick_ = (highTick_ + 1) % highCapacity_;

    uint32_t currenttime = getCurrentTime();
    while (!highWheel_[highTick_].empty()) {
        auto& task = highWheel_[highTick_].front();
        uint32_t elapsedTime = currenttime - task->getStartTime();
        uint32_t remainingTime = task->getTimeout() - elapsedTime;

        int newPos = (midTick_ + remainingTime / lowCapacity_) % midCapacity_;
        midWheel_[newPos].push_back(task);
        highWheel_[highTick_].pop_front();
    }
}

TaskPtr TimerWheel::createTask(uint64_t id, uint32_t timeout, const TaskFunc& cb) {
    auto it = timerTasks_.find(id);
    if (it != timerTasks_.end()) {
        TaskPtr existingTask = it->second.lock();
        if (existingTask) {
            return existingTask;
        }
    }

    TaskPtr newTask = make_shared<TimerTask>(id, this->getCurrentTime(), timeout, cb);
    timerTasks_[id] = TaskWeak(newTask);
    newTask->setRelease(bind(&TimerWheel::RemoveTimer, this, id));

    return newTask;
}

void TimerWheel::addTimerToLowWheel(uint64_t id, uint32_t timeout, const TaskFunc& cb) {
    TaskPtr tp = createTask(id, timeout, cb);
    int pos = (lowTick_ + timeout) % lowCapacity_;
    lowWheel_[pos].push_back(tp);
}

void TimerWheel::addTimerToMidWheel(uint64_t id, uint32_t timeout, const TaskFunc& cb) {
    TaskPtr tp = createTask(id, timeout, cb);
    int minutePos = (midTick_ + timeout / lowCapacity_) % midCapacity_;
    midWheel_[minutePos].push_back(tp);
}

void TimerWheel::addTimerToHighWheel(uint64_t id, uint32_t timeout, const TaskFunc& cb) {
    TaskPtr tp = createTask(id, timeout, cb);
    int hourPos = (highTick_ + timeout / (lowCapacity_ * midCapacity_)) % highCapacity_;
    highWheel_[hourPos].push_back(tp);
}

void TimerWheel::RemoveTimer(uint64_t id) {
    auto it = timerTasks_.find(id);
    if (it != timerTasks_.end()) {
        timerTasks_.erase(it);
    }
}
