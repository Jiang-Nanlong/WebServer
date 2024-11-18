#include "../include/Poller.h"

const int Poller::KNew = -1;
const int Poller::KAdded = 1;
const int Poller::KDeleted = 2;

Poller::Poller() :
    epollFd_(epoll_create(1)),
    events_(kInitEventListSize) {
    if (epollFd_ < 0) {
        LOG(ERROR, "create epoll instance failed");
        exit(1);
    }
}

uint32_t Poller::getChannelNum() const {
    return channels_.size();
}

void Poller::updateChannel(Channel* ch) {
    /* if (!hasChannel(ch)) {
        update(ch, EPOLL_CTL_ADD);
        channels_[ch->getFd()] = ch;
    }
    else
        update(ch, EPOLL_CTL_MOD); */

    int status = ch->getStatus();
    if (status == KNew || status == KDeleted) {
        update(ch, EPOLL_CTL_ADD);
        channels_[ch->getFd()] = ch;
        ch->setStatus(KAdded);
    }
    else {
        update(ch, EPOLL_CTL_MOD);
    }
}

void Poller::removeChannel(Channel* ch) {
    /* if (hasChannel(ch)) {
        update(ch, EPOLL_CTL_DEL);
        channels_.erase(ch->getFd());
    } */
    int status = ch->getStatus();
    if (status == KAdded) {
        update(ch, EPOLL_CTL_DEL);
        channels_.erase(ch->getFd());
        ch->setStatus(KDeleted);
    }
}

void Poller::update(Channel* ch, int op) {
    int fd = ch->getFd();
    struct epoll_event ev;
    ev.events = ch->getEvents();
    ev.data.ptr = ch;
    int ret = epoll_ctl(epollFd_, op, fd, &ev);
    if (ret < 0) {
        if (op == EPOLL_CTL_DEL)
            LOG(ERROR, "epoll_ctl delete failed");
        else
            LOG(FATAL, "epoll_ctl operator failed");
    }
}

bool Poller::hasChannel(Channel* ch) {
    auto it = channels_.find(ch->getFd());
    return it != channels_.end() && it->second == ch;
}

// muduo库中这里要返回一个时间戳，在eventloop中要用于channel的handleevent函数，在handleevent函数中主要用于读回调函数
void Poller::poll(int TimeOuts, vector<Channel*>& ChannelList) {
    int nfds = epoll_wait(epollFd_, &*events_.begin(), static_cast<int>(events_.size()), TimeOuts);
    if (nfds < 0) {
        if (errno == EINTR) {
            LOG(INFO, "epoll wait EINTR");
            return;
        }
        LOG(ERROR, "epoll wait failed: %s", strerror(errno));
        exit(1);
    }
    else if (nfds == 0) {
        LOG(INFO, "nothing happened");
    }
    else {
        for (int i = 0;i < nfds;++i) {
            Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
            ch->setRevents(events_[i].events);
            ChannelList.push_back(ch);
        }
        if (nfds == static_cast<int>(events_.size()))  // 这个地方倒是不用担心有事件这次每次处理，默认采用水平触发，下一次还会再就绪
            // 不过使用vector可以避免一开始就分配大段空间，这在高并发的时候会导致内存消耗严重
            events_.resize(nfds * 2);
    }
}
