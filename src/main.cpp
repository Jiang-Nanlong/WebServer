#include "TcpServer.h"

int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 8081);
    TcpServer server(&loop, addr, "myserver");
    server.setThreadNum(3);
    server.start();
    loop.loop();

    return 0;
}