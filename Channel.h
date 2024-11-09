#pragma once

#include <stdint.h>
#include <functional>
#include <sys/epoll.h>
using namespace std;

// 一个Channel对象代表着一个可被epoll监听的文件描述符，之所以使用Channel类是为了在向epoll中注册时使用epoll_event.data.ptr可以携带更多文件描述符的信息

class EventLoop;

class Channel {
private:
    int fd;
    bool InEpoll = false;
    uint32_t events = 0;
    uint32_t revents = 0;
    unique_ptr<EventLoop> lp;   // 只需要修改每个Channel实例就能从EventLoop到Poller都修改

    using EventCallback = function <void()>;
    EventCallback _read_callback;
    EventCallback _write_callback;
    EventCallback _error_callback;
    EventCallback _close_callback;
    EventCallback _event_callback;
public:
    Channel(int fd, EventLoop* loop) :fd(fd), lp(loop), _read_callback(nullptr), _write_callback(nullptr), _error_callback(nullptr), _close_callback(nullptr), _event_callback(nullptr) {}

    ~Channel() {}

    int GetFd();

    void SetInEpoll(bool flag);

    bool GetInEpoll();

    void Update();

    uint32_t GetEvents();

    void enableReading();

    void disableReading();

    void enableWriting();

    void disableWriting();

    void disableAll();

    uint32_t GetRevents();

    void SetReadCallback(EventCallback fn);

    void SetWriteCallback(EventCallback fn);

    void SetErrorCallback(EventCallback fn);

    void SetCloseCallback(EventCallback fn);

    void SetEventCallback(EventCallback fn);

    void HandleEvent();
};