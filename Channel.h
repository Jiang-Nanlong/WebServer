#pragma once

#include <stdint.h>
#include <functional>
#include <sys/epoll.h>
using namespace std;

// 一个Channel对象代表着一个可被epoll监听的文件描述符，之所以使用Channel类是为了在向epoll中注册时使用epoll_event.data.ptr可以携带更多文件描述符的信息

class Channel {
private:
    int fd;
    bool InEpoll = false;   // 是否已经加入到epoll例程中
    uint32_t events = 0;    // 文件描述符fd想要监听的事件
    uint32_t revents = 0;   // Poller返回的具体发生的事件

    using EventCallback = function <void()>;
    EventCallback _read_callback;
    EventCallback _write_callback;
    EventCallback _error_callback;
    EventCallback _close_callback;
    EventCallback _event_callback;
public:
    Channel(int fd) :fd(fd), _read_callback(nullptr), _write_callback(nullptr), _error_callback(nullptr), _close_callback(nullptr), _event_callback(nullptr) {}

    ~Channel() {}

    int GetFd();

    void SetInEpoll(bool flag);

    bool GetInEpoll();

    uint32_t GetEvents();

    void SetEvents(uint32_t event);

    uint32_t GetRevents();

    void SetRevents(uint32_t event);

    void SetReadCallback(EventCallback fn);

    void SetWriteCallback(EventCallback fn);

    void SetErrorCallback(EventCallback fn);

    void SetCloseCallback(EventCallback fn);

    void SetEventCallback(EventCallback fn);

    void HandleEvent();     // epoll_wait返回时调用对应的函数
};