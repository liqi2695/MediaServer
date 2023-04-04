#include <iostream>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if(argc <= 2) {
        std::cout << "input ip and port" << std::endl;
        exit(-1);
    }
    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clisock;
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    bzero(&cliaddr, sizeof(cliaddr));
    clisock = socket(AF_INET, SOCK_STREAM, 0);

    cliaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &cliaddr.sin_addr);
    cliaddr.sin_port = htons(port);


    int ret = connect(clisock, (struct sockaddr*)&cliaddr, len);
    if(ret < 0) {
        std::cout << "connect error" << std::endl;
    }

}