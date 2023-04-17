#ifndef SOCKET_H_
#define SOCKET_H_

#include <cstdint>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Address.h"

//这里为什么需要
class InetAddress;

/*不提供fd创建方法，所以创建UDP还是TCP由你决定*/
class Socket {
public:
    explicit Socket(int sockfd) : _sockfd(sockfd) { }
    ~Socket();

    int fd() const { return  _sockfd; }
    bool getTcpInfo(struct tcp_info*) const;
    

    void bindAddress(InetAddress& localaddr);

    void listenClient();

    int acceptCli(InetAddress* peeraddr);

    void shutDownWrite();



private:
    const int _sockfd;
};







#endif