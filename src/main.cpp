#include "TcpServer.h"

class Server {
private:
    EventLoop* loop;
    TcpServer serv;
public:
    Server(EventLoop* loop, InetAddress& addr, const string& name) :
        serv(loop, addr, name),
        loop(loop)
    {
        serv.setConnectionCallback(bind(&Server::connectionCallback_, this, std::placeholders::_1));
        serv.setMessageCallback(bind(&Server::readCallback_, this, std::placeholders::_1, std::placeholders::_2));
        serv.setThreadNum(3);
    }

    void start() {
        serv.start();
        loop->loop();
    }

private:
    void connectionCallback_(const ConnectionPtr& conn) {
        if (conn->isConnected())
            LOG(INFO, "connection UP");
        else
            LOG(INFO, "connection DOWN");
    }

    void readCallback_(const ConnectionPtr& conn, Buffer* buf) {
        string str = buf->readAsStringAndPop(buf->readAbleSize());
        conn->send(str);
        //conn->shutdown();
    }
};


int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 8081);
    Server server(&loop, addr, "myserver");
    server.start();

    return 0;
}