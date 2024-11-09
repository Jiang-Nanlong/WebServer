#pragma once

#include <sys/epoll.h>
#include <vector>
#include "Channel.h"
#include <cstring>
#include <unistd.h>
#include "Log.h"
using namespace std;

#define MAX_EVENTS 1024

// Poller模块用来监听事件

class Poller {
private:
    int epollfd;    // epoll例程
    struct epoll_event evs[MAX_EVENTS];  // 当前epoll监听的epoll_event

public:
    Poller() {
        epollfd = epoll_create(1);
        if (epollfd < 0) {
            LOG(ERROR, "create epoll instance failed");
            exit(1);
        }
    }

    ~Poller() {
        close(epollfd);
    }

    void UpdateChannel(Channel* ch, int op);

    void Poll(vector<Channel*>& ChannelList);
};