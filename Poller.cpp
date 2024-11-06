#include "Poller.h"

void Poller::UpdateChannel(Channel* ch, int op) {
    int fd = ch->GetFd();
    struct epoll_event event;
    event.events = ch->GetEvents();
    event.data.ptr = ch;

    if (op == EPOLL_CTL_ADD) {
        if (ch->GetInEpoll() == false) {
            if (epoll_ctl(epollfd, op, fd, &event) < 0) {
                // 日志
                return;
            }
            ch->SetInEpoll(true);
        }
    }
    else if (op == EPOLL_CTL_DEL) {
        if (ch->GetInEpoll() == true) {
            if (epoll_ctl(epollfd, op, fd, &event) < 0) {
                // 日志
                return;
            }
            ch->SetInEpoll(false);
        }
    }
    else if (op == EPOLL_CTL_MOD) {
        if (ch->GetInEpoll() == true) {
            if (epoll_ctl(epollfd, op, fd, &event) < 0) {
                // 日志
                return;
            }
        }
    }
}

vector<Channel*> Poller::Poll() {
    memset(evs, 0, sizeof(epoll_event) * MAX_EVENTS);
    int nfds = epoll_wait(epollfd, evs, MAX_EVENTS, -1);
    if (nfds < 1) {
        exit(1);
    }

    vector<Channel*> res;
    for (int i = 0;i < nfds;++i) {
        Channel* ch = (Channel*)evs[i].data.ptr;
        if (ch->GetInEpoll()) {
            ch->SetRevents(evs[i].events);
            res.push_back(ch);
        }
    }
    return res;
}
