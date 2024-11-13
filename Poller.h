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
    int epollFd_;
    using ChannelMap = unordered_map<int, Channel*>;
    ChannelMap channels_;

    using EventList = vector<struct epoll_event>;
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