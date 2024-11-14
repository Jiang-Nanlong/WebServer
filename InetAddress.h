#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
using namespace std;

// 封装sockaddr_in
class InetAddress {
private:
    sockaddr_in addr_;
public:
    explicit InetAddress(string ip = "127.0.0.1", uint16_t port = 0);

    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}

    string toIp() const;

    string toIpPort() const;

    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const { return &addr_; }

    void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }

};