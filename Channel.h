#pragma once

#include <stdint.h>
#include <functional>
#include <sys/epoll.h>
#include <memory>

#include "noncopyable.h"
#include "Callbacks.h"

using namespace std;


class EventLoop;

class Channel :noncopyable {
private:
    int fd_;
    uint32_t events_ = 0;
    uint32_t revents_ = 0;
    unique_ptr<EventLoop> lp_;   // 只需要修改每个Channel实例就能从EventLoop到Poller都修改

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;

    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    int status_;  // 在poller中的状态 -1,1,2，可以在Channel中直接看到它在poller中的状态，而不用经过Eventloop

public:
    Channel(int fd, EventLoop* loop);

    ~Channel() = default;

    int getFd();

    void setStatus(int flag);

    int getStatus();

    void update();

    void remove();

    uint32_t getEvents();

    void enableReading();

    void disableReading();

    void enableWriting();

    void disableWriting();

    void disableAll();

    uint32_t getRevents();

    void setRevents(int event);

    void setReadCallback(EventCallback fn);

    void setWriteCallback(EventCallback fn);

    void setErrorCallback(EventCallback fn);

    void setCloseCallback(EventCallback fn);

    bool isNoneEvent() const { return events_ == KNoneEvent; }

    bool isWriteAble() const { return events_ & KWriteEvent; }

    bool isReadAble() const { return events_ & KReadEvent; }

    void handleEvent();
};