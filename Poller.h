/*
 * @Author: Cao Menglong
 * @Date: 2024-11-11 17:32:24
 * @LastEditTime: 2024-11-11 21:45:24
 * @Description:
 */
#pragma once

#include <sys/epoll.h>
#include <vector>
#include "Channel.h"
#include <cstring>
#include <unistd.h>
#include "Log.h"
#include "noncopyable.h"
#include <unordered_map>
using namespace std;

#define MAX_EVENTS 1024

class Poller :noncopyable {
private:
    int epollfd;
    struct epoll_event evs[MAX_EVENTS];  // 当前epoll监听的epoll_event
    using ChannelMap = unordered_map<int, Channel*>;
    ChannelMap channels_;

    void Update(Channel* ch, int op);
public:
    Poller();

    ~Poller() { close(epollfd); }

    void UpdateChannel(Channel* ch);

    void RemoveChannel(Channel* ch);

    bool hasChannel(Channel* ch);

    void Poll(vector<Channel*>& ChannelList);
};