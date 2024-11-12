#include "Poller.h"

const int Poller::KNew = -1;
const int Poller::KAdded = 1
const int Poller::KDeleted = 2;

Poller::Poller() :
    epollfd(epoll_create(1)),
    events_(kInitEventListSize) {
    if (epollfd < 0) {
        LOG(ERROR, "create epoll instance failed");
        exit(1);
    }
}

void Poller::UpdateChannel(Channel* ch) {
    /* if (!hasChannel(ch)) {
        Update(ch, EPOLL_CTL_ADD);
        channels_[ch->GetFd()] = ch;
    }
    else
        Update(ch, EPOLL_CTL_MOD); */

    int status = ch->GetStatus();
    if (status == KNew || status == KDeleted) {
        Update(ch, EPOLL_CTL_ADD);
        channels_[ch->GetFd()] = ch;
        ch->SetStatus(KAdded);
    }
    else {
        Update(ch, EPOLL_CTL_MOD);
    }
}

void Poller::RemoveChannel(Channel* ch) {
    /* if (hasChannel(ch)) {
        Update(ch, EPOLL_CTL_DEL);
        channels_.erase(ch->GetFd());
    } */
    int status = ch->GetStatus();
    if (status == KAdded) {
        Update(ch, EPOLL_CTL_DEL);
        channels_.erase(ch->GetFd());
        ch->SetStatus(KDeleted);
    }
}

void Poller::Update(Channel* ch, int op) {
    int fd = ch->GetFd();
    struct epoll_event ev;
    ev.events = ch->GetEvents();
    ev.data.ptr = ch;
    int ret = epoll_ctl(epollfd, op, fd, &ev);
    if (ret < 0) {
        if (op == EPOLL_CTL_DEL)
            LOG(ERROR, "epoll_ctl delete failed");
        else
            LOG(FATAL, "epoll_ctl operator failed");
    }
}

bool Poller::hasChannel(Channel* ch) {
    auto it = channels_.find(ch->GetFd());
    return it != channels_.end() && it->second == ch;
}

// muduo库中这里要返回一个时间戳，在eventloop中要用于channel的handleevent函数，在handleevent函数中主要用于读回调函数
void Poller::Poll(int TimeOuts, vector<Channel*>& ChannelList) {
    int nfds = epoll_wait(epollfd, &*events_.begin(), static_cast<int>(events_.size()), TimeOuts);
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
            ch->SetRevents(events_[i].events);
            ChannelList.push_back(ch);
        }
        if (nfds == static_cast<int>(events_.size()))  // 这个地方倒是不用担心有事件这次每次处理，默认采用水平触发，下一次还会再就绪
            // 不过使用vector可以避免一开始就分配大段空间，这在高并发的时候会导致内存消耗严重
            events_.resize(nfds * 2);
    }
}
