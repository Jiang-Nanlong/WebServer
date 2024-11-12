/*
 * @Author: Cao Menglong
 * @Date: 2024-11-11 17:32:24
 * @LastEditTime: 2024-11-11 20:19:08
 * @Description:
 */
#include "Channel.h"

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::KWriteEvent = EPOLLOUT;

int Channel::GetFd() {
    return fd;
}

void Channel::Update() {
    lp->UpdateChannel(this);
}

void Channel::Remove() {
    lp->RemoveChannel(this);
}

void Channel::enableReading() { events |= KReadEvent; Update(); }

void Channel::disableReading() { events &= ~KReadEvent; Update(); }

void Channel::enableWriting() { events |= KWriteEvent; Update(); }

void Channel::disableWriting() { events &= ~KWriteEvent; Update(); }

void Channel::disableAll() { events = KNoneEvent; Update(); }

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

void Channel::SetRevents(int event) {
    revents = event;
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

void Channel::HandleEvent() {
    if (revents & EPOLLHUP && !(revents & EPOLLIN)) {
        if (_close_callback)_close_callback();
    }
    else if (revents & (EPOLLIN | EPOLLRDHUP | EPOLLPRI)) {
        if (_read_callback)_read_callback();
    }
    else if (revents & EPOLLOUT) {
        if (_write_callback)_write_callback();
    }
    else if (revents & EPOLLERR) {
        if (_error_callback)_error_callback();
        return;
    }
}
