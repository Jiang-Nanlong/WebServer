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
    using ChannelMap = unordered_map<int, Channel*>;
    ChannelMap channels_;

    using EventList = vector<struct epoll_event>;
    EventList events_;

    static const int kInitEventListSize = 16;   // events_的初始大小

    static const int KNew;
    static const int KAdded;
    static const int KDeleted;

    void Update(Channel* ch, int op);
public:
    Poller();

    ~Poller() { close(epollfd); }

    void UpdateChannel(Channel* ch);

    void RemoveChannel(Channel* ch);

    bool hasChannel(Channel* ch);

    void Poll(int TimeOuts, vector<Channel*>& ChannelList);
};