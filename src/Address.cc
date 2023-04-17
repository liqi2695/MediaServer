#include "Address.h"



InetAddress::InetAddress(uint16_t port) {
    bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _addr.sin_port = htons(port);
}

InetAddress::InetAddress(String ip, uint16_t port) {
    bzero(&_addr, sizeof(_addr));

    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip.get_str());
    _addr.sin_port = htons(port);
}
