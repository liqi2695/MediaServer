#include "Socket.h"
#include "Address.h"


int main(int argc, char* argv[]) 
{
    uint16_t port = atoi(argv[1]);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    Socket servsocket(fd);

    InetAddress address(port);

    servsocket.bindAddress(address);
    servsocket.listenClient();

    while(1) {
        InetAddress cliaddress;
        servsocket.acceptCli(&cliaddress);

        



    }
}