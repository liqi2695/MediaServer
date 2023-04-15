#include "Socket.h"
#include "error_help.h"


int TCPSocket::createSocket() {
    int ret = socket(AF_INET, SOCK_STREAM, 0);
    if(ret < 0) {
        errMsg("create socket error");
    }
    return ret;
}

void TCPSocket::Bind(int fd) {
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(fd, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    if(bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        errMsg("bind error");
    }
}

int TCPSocket::Accept(int fd, char* cliIP, int* cliPort) {
    socklen_t len = sizeof(clitaddr);
    bzero(&clitaddr, sizeof(clitaddr));
    int clifd = accept(fd, (struct sockaddr*)&clitaddr, &len);
    if(clifd < 0) {
        errMsg("accept error");
    }
    
    strcpy(cliIP, inet_ntoa(clitaddr.sin_addr));
    *cliPort = ntohs(clitaddr.sin_port);

    return clifd;
}