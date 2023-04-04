#include <iostream>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


int main(int argc, char *argv[]) {
    //接收进程传入参数
    if(argc <= 2) {
        std::cout << "input ip and port" << std::endl;
        exit(-1);
    }
    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //开始设置server的fd 
    int servfd;
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servfd = socket(PF_INET, SOCK_STREAM, 0);
    if(servfd < 0) {
        std::cout << "create socket failed!" << std::endl;
    }
    //bind listen accept read send

    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    int ret = bind(servfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0) {
        std::cout << "bind error" << std::endl;
    }
    listen(servfd, 10);

    while(1) {
        int clitfd;
        struct sockaddr_in cliaddr;
        bzero(&cliaddr, sizeof(cliaddr));
        socklen_t len = sizeof(cliaddr);
        clitfd = accept(servfd, (struct sockaddr*)&cliaddr, &len);
        if(clitfd < 0) {
            std::cout << "accept erroe" << std::endl;
            exit(-1);
        }
        char buf[100];
        while(1) {
            
            bzero(&buf, sizeof(buf));
            recv(clitfd, buf, 100 - 1, 0);
            
            if(buf == "quit") exit(-1);
            std::cout << buf << std::endl;
        }
        close(clitfd);
    }


} 
