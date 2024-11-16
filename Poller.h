#pragma once

#include <sys/epoll.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <unordered_map>

#include "Log.h"
#include "noncopyable.h"
#include "Channel.h"

using namespace std;

#define MAX_EVENTS 1024

class Poller :noncopyable {
    using ChannelMap = unordered_map<int, Channel*>;
    using EventList = vector<struct epoll_event>;
private:
    int epollFd_;
    ChannelMap channels_;
    EventList events_;
    static const int kInitEventListSize = 16;   // events_的初始大小

    static const int KNew;
    static const int KAdded;
    static const int KDeleted;

    void update(Channel* ch, int op);
public:
    Poller();

    ~Poller() { close(epollFd_); }

    void updateChannel(Channel* ch);

    void removeChannel(Channel* ch);

    bool hasChannel(Channel* ch);

    void poll(int TimeOuts, vector<Channel*>& ChannelList);
};