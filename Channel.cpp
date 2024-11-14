#include "Channel.h"

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::KWriteEvent = EPOLLOUT;

Channel::Channel(int fd, EventLoop* loop) :
    fd_(fd),
    lp_(loop),
    status(-1),
    readCallback_(nullptr),
    writeCallback_(nullptr),
    errorCallback_(nullptr),
    closeCallback_(nullptr) {
    lp_->updateChannel(this);   // Channel刚创建就注册到对应的eventloop上
}

int Channel::getFd() {
    return fd;
}

void Channel::setStatus(int flag) {
    status = flag;
}

int Channel::getStatus() {
    return status;
}

void Channel::update() {
    lp->UpdateChannel(this);
}

void Channel::remove() {
    lp->RemoveChannel(this);
}

void Channel::enableReading() { events_ |= KReadEvent; update(); }

void Channel::disableReading() { events_ &= ~KReadEvent; update(); }

void Channel::enableWriting() { events_ |= KWriteEvent; update(); }

void Channel::disableWriting() { events_ &= ~KWriteEvent; update(); }

void Channel::disableAll() { events_ = KNoneEvent; update(); }

uint32_t Channel::getEvents() {
    return events_;
}

// 上边有针对读写的事件更细分的设置
/*
void Channel::SetEvents(uint32_t event) {
    events_ |= event;
}
*/

uint32_t Channel::getRevents() {
    return revents_;
}

void Channel::setRevents(int event) {
    revents_ = event;
}

void Channel::setReadCallback(EventCallback fn) {
    readCallback_ = fn;
}

void Channel::setWriteCallback(EventCallback fn) {
    writeCallback_ = fn;
}

void Channel::setErrorCallback(EventCallback fn) {
    errorCallback_ = fn;
}

void Channel::setCloseCallback(EventCallback fn) {
    closeCallback_ = fn;
}

void Channel::handleEvent() {
    if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
        if (closeCallback_)closeCallback_();
    }
    else if (revents_ & (EPOLLIN | EPOLLRDHUP | EPOLLPRI)) {
        if (readCallback_)readCallback_();
    }
    else if (revents_ & EPOLLOUT) {
        if (writeCallback_)writeCallback_();
    }
    else if (revents_ & EPOLLERR) {
        if (errorCallback_)errorCallback_();
        return;
    }
}
