#include "Socket.h"
#include "error_help.h"

Socket::~Socket() {
    close(_sockfd);
}

bool Socket::getTcpInfo(struct tcp_info* tcpin) const {
    socklen_t len = sizeof(*tcpin);
    bzero(tcpin, len);
    return getsockopt(_sockfd, SOL_TCP, TCP_INFO, tcpin, &len);
}

void Socket::bindAddress(InetAddress& localaddr) {
    struct sockaddr_in serveraddr = localaddr.getSockAddr();
    if(bind(_sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        errMsg("Socket bind error");
    }
} 

void Socket::listenClient() {
    if(listen(_sockfd, SOMAXCONN) < 0) {
        errMsg("listen error");
    }
}


int Socket::acceptCli(InetAddress* peeraddr) {
    struct sockaddr_in cliaddr = peeraddr->getSockAddr();
    socklen_t len = sizeof(cliaddr);
    int ret = 0;
    if( (ret = accept(_sockfd, (struct sockaddr*)&cliaddr, &len)) < 0) {
        errMsg("Accept error");
    }
    return ret;
}

