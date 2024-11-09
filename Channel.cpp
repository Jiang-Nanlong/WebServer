#include "Channel.h"

int Channel::GetFd() {
    return fd;
}

void Channel::SetInEpoll(bool flag) {
    InEpoll = flag;
}

bool Channel::GetInEpoll()
{
    return InEpoll;
}

void Channel::Update() {
    lp->ModifyChannel(this);
}

void Channel::enableReading() { events |= EPOLLIN; Update(); }

void Channel::disableReading() { events &= ~EPOLLIN; Update(); }

void Channel::enableWriting() { events |= EPOLLOUT; Update(); }

void Channel::disableWriting() { events &= ~EPOLLOUT; Update(); }

void Channel::disableAll() { events = 0; Update(); }

uint32_t Channel::GetEvents() {
    return events;
}

/*
void Channel::SetEvents(uint32_t event) {
    events |= event;
}
*/

uint32_t Channel::GetRevents() {
    return revents;
}

void Channel::SetReadCallback(EventCallback fn) {
    _read_callback = fn;
}

void Channel::SetWriteCallback(EventCallback fn) {
    _write_callback = fn;
}

void Channel::SetErrorCallback(EventCallback fn) {
    _error_callback = fn;
}

void Channel::SetCloseCallback(EventCallback fn) {
    _close_callback = fn;
}

void Channel::SetEventCallback(EventCallback fn) {
    _event_callback = fn;
}

void Channel::HandleEvent() {
    if (revents & EPOLLHUP && !(revents & EPOLLIN))
        _close_callback();
    else if (revents & (EPOLLIN | EPOLLRDHUP | EPOLLPRI))
        _read_callback();
    else if (revents & EPOLLOUT)
        _write_callback();
    else if (revents & EPOLLERR) {
        _error_callback();
        return;
    }
    _event_callback();
}
