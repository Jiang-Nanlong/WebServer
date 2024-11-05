#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

#define LISTEN_NUM 10

class Socket
{
private:
    int sockfd;
public:
    Socket() :sockfd(-1) {}

    Socket(int fd) :sockfd(fd) {}

    ~Socket() { Close(); };

    int GetFd() { return sockfd; }

    bool Create() ;

    bool Bind(const string& ip, uint16_t port);

    bool Listen(int num = LISTEN_NUM);

    bool Connect(const string& ip, uint16_t port);

    int Accept();

    ssize_t Recv(void* buf, size_t len, int flag = 0);

    ssize_t NonBlockRecv(void* buf, size_t len);

    ssize_t Send(void* buf, size_t len, int flag = 0);

    ssize_t NonBlockSend(void* buf, size_t len);

    void Close();

    // 当前sockfd是服务端套接字
    bool CreateServer(const string& ip = "0.0.0.0", uint16_t port, bool block_flag = false);

    // 当前sockfd是客户端套接字
    bool CreateClient(const string& ip, uint16_t port);

    //设置套接字开启地址端口重用
    void ReuseAddress();
    
    //设置套接字为非阻塞
    void NonBlock();
};
