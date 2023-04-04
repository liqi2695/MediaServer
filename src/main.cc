#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "Socket.h"

#define SERVER_PORT    8080
#define SERVER_RTP_PORT 55534
#define SERVER_RTCP_PORT 55535

int main(int argc, char* argv[]) {
    
    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    
    TCPSocket sock(ip, port);
    int servfd = sock.createSocket();

    sock.Bind(servfd);
    listen(servfd, 10);

    while(1) {
        int clitsock = sock.Accept(servfd);
    }
}
