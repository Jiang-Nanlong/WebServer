#include "Poller.h"


Poller::Poller() {
    epollfd = epoll_create(1);
    if (epollfd < 0) {
        LOG(ERROR, "create epoll instance failed");
        exit(1);
    }
}

void Poller::UpdateChannel(Channel* ch) {
    if (!hasChannel(ch)) {
        Update(ch, EPOLL_CTL_ADD);
        channels_[ch->GetFd()] = ch;
    }
    else
        Update(ch, EPOLL_CTL_MOD);
}

void Poller::RemoveChannel(Channel* ch) {
    if (hasChannel(ch)) {
        Update(ch, EPOLL_CTL_DEL);
        channels_.erase(ch->GetFd());
    }
}

void Poller::Update(Channel* ch, int op) {
    int fd = ch->fd;
    struct epoll_event ev;
    ev.events = ch->GetEvents();
    ev.data.ptr = ch;
    int ret = epoll_ctl(epollfd, op, fd, &ev);
    if (ret < 0) {
        LOG(ERROR, "epoll_ctl operate failed");
    }
}

bool Poller::hasChannel(Channel* ch) {
    auto it = channels_.find(ch->GetFd());
    return it != channels_.end() && it->second == ch;
}

void Poller::Poll(vector<Channel*>& ChannelList) {
    memset(evs, 0, sizeof(epoll_event) * MAX_EVENTS);
    int nfds = epoll_wait(epollfd, evs, MAX_EVENTS, -1);
    if (nfds < 0) {
        if (errno == EINTR) {
            LOG(INFO, "epoll wait EINTR");
            return;
        }
        LOG(ERROR, "epoll wait failed: %s", strerror(errno));
        exit(1);
    }
    for (int i = 0;i < nfds;++i) {
        Channel* ch = (Channel*)evs[i].data.ptr;
        if (hasChannel(ch)) {
            ch->SetRevents(evs[i].events);
            ChannelList.push_back(ch);
        }
    }
}
