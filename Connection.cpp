#include "Connection.h"

Connection::Connection(EventLoop* loop,
    const string& name,
    int sockfd,
    const InetAddress& localaddr,
    const InetAddress& remoteaddr) :
    loop_(loop),
    name_(name),
    socket_(new Socket(fd)),
    channel_(new Channel(fd, loop_)),
    reading_(true),
    state_(kConnecting),
    localAddr_(localaddr),
    remoteAddr_(remoteaddr),
    highWaterMark_(64 * 1024 * 1024)  // 64M
{
}