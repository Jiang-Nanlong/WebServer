#pragma once

#include "Callbacks.h"

class TimerTask {
public:
    TimerTask(uint64_t id, uint32_t currenttime, uint32_t timeout, const TaskFunc& cb)
        : id_(id),
        startTime_(currenttime),
        timeout_(timeout),
        isCancel_(false),
        taskFunc_(cb) {
    }

    void setRelease(const ReleaseFunc& cb) {
        releaseFunc_ = cb;
    }

    uint32_t getTimeout() const { return timeout_; }
    uint32_t getStartTime() const { return startTime_; }
    uint64_t getTaskId() const { return id_; }

    void cancel() { isCancel_ = true; }

    ~TimerTask() {
        if (!isCancel_ && taskFunc_) {
            taskFunc_();
        }
        if (releaseFunc_) {
            releaseFunc_(id_);
        }
    }

private:
    uint64_t id_;     // 这里的id可以用每个Channel的sockfd
    uint32_t startTime_;
    uint32_t timeout_;
    bool isCancel_;
    TaskFunc taskFunc_;
    ReleaseFunc releaseFunc_;
};