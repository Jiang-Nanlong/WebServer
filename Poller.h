#pragma once

#include <sys/epoll.h>
#include <vector>
#include "Channel.h"
#include <cstring>
#include <unistd.h>
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
            exit(1);
        }
    }

    ~Poller() {
        close(epollfd);
    }

    void UpdateChannel(Channel* ch, int op);   // 向epoll例程中添加，修改或删除要监听的文件描述符及其属性

    vector<Channel*> Poll();    // 开始监听注册在epoll例程上的事件
};