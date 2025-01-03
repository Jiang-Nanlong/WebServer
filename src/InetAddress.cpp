#include "../include/InetAddress.h"

InetAddress::InetAddress(string ip, uint16_t port) {
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

string InetAddress::getIp() const {
    char buf[64] = { 0 };
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

uint16_t InetAddress::getPort() const {
    return ntohs(addr_.sin_port);
}

string InetAddress::getIpPort() const {
    char buf[64] = { 0 };
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}
