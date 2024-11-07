#include "EventLoop.h"



void EventLoop::loop() {
    while (1) {
        vector<Channel*> res = poller->Poll();
        for (auto it : res)
            it->HandleEvent();

    }
}