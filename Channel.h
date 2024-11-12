/*
 * @Author: Cao Menglong
 * @Date: 2024-11-11 17:32:24
 * @LastEditTime: 2024-11-11 20:26:30
 * @Description:
 */
#pragma once

#include <stdint.h>
#include <functional>
#include <sys/epoll.h>
#include "noncopyable.h"
using namespace std;

// 一个Channel对象代表着一个可被epoll监听的文件描述符，之所以使用Channel类是为了在向epoll中注册时使用epoll_event.data.ptr可以携带更多文件描述符的信息

class EventLoop;

class Channel :noncopyable {
private:
    int fd;
    uint32_t events = 0;
    uint32_t revents = 0;
    unique_ptr<EventLoop> lp;   // 只需要修改每个Channel实例就能从EventLoop到Poller都修改

    using EventCallback = function <void()>;
    EventCallback _read_callback;
    EventCallback _write_callback;
    EventCallback _error_callback;
    EventCallback _close_callback;

    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

public:
    Channel(int fd, EventLoop* loop) :
        fd(fd),
        lp(loop),
        _read_callback(nullptr),
        _write_callback(nullptr),
        _error_callback(nullptr),
        _close_callback(nullptr) {
        lp->AddChannel(this);
    }

    ~Channel() {}

    int GetFd();

    void SetInEpoll(bool flag);

    bool GetInEpoll();

    void Update();

    void Remove();

    uint32_t GetEvents();

    void enableReading();

    void disableReading();

    void enableWriting();

    void disableWriting();

    void disableAll();

    uint32_t GetRevents();

    void SetRevents(int event);

    void SetReadCallback(EventCallback fn);

    void SetWriteCallback(EventCallback fn);

    void SetErrorCallback(EventCallback fn);

    void SetCloseCallback(EventCallback fn);

    bool isNoneEvent() const { return events == KNoneEvent; }

    bool isWriteAble() const { return events & KWriteEvent; }

    bool isReadAble() const { return events & KReadEvent; }

    void HandleEvent();
};