#pragma once

#include <thread>
#include "Poller.h"
#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <sys/eventfd.h>

using namespace std;

class EventLoop {
private:
    const pid_t threadId;
    int EventFd;
    shared_ptr<Poller> poller;
    using Functor = function<void()>;
    vector<Functor> Tasks;
    mutex mtx;

    static int CreateEventFd() {
        int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (fd < 0) {
            exit(1);
        }
        return fd;
    }

public:
    EventLoop() :threadId(std::this_thread::get_id()), EventFd(CreateEventFd()), poller(new Poller()) {}

    void loop();
};