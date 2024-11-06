#include "Channel.h"

int Channel::GetFd() {
    return fd;
}

void Channel::SetInEpoll(bool flag) {
    InEpoll = flag;
}

bool Channel::GetInEpoll()
{
    return InEpoll;
}

uint32_t Channel::GetEvents() {
    return events;
}

void Channel::SetEvents(uint32_t event) {
    events |= event;
}

uint32_t Channel::GetRevents() {
    return revents;
}

void Channel::SetRevents(uint32_t event) {
    revents = event;
}

void Channel::HandleEvent() {
}


