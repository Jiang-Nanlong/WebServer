#pragma once

#include <sys/epoll.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <unordered_map>
#include <memory>

#include "Log.h"
#include "noncopyable.h"
#include "Channel.h"

using namespace std;

#define MAX_EVENTS 1024

class Poller :noncopyable {
    using ChannelMap = unordered_map<int, Channel*>;
    // 这里直接使用Channel*来指向一个Channel，而不是使用智能指针是因为Poller不负责Channel的生命周期的管理，在上层的Connection中负责管理。
    // Connection中使用unique_ptr来管理一个Channel对象，而一个Connection对象的生命周期和对应的Channel的生命周期是相同的，当一个Connection断开时，会从底层Poller中移除对Channel的监听，
    // 然后Connection对象析构时，智能指针会自动释放它管理的Channel对象。
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