#ifndef SOCKET_H_
#define SOCKET_H_

#include <cstdint>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// 网络套接字的基类，可被TCP,UDP继承
class Socket {
private:
    //socket创建需要的变量
public:
    virtual int createSocket();
    virtual void Bind(int fd);
    virtual int Accept(int fd);

    // virtual void sendMsg(std::string &str);
    // virtual std::string recvMsg();
};

class TCPSocket {
private:
    const char* ip;
    uint16_t port;
public:    
    struct sockaddr_in servaddr;
    struct sockaddr_in clitaddr;

public:
    TCPSocket(const char* IP, uint16_t Port)
        :ip(IP), port(Port) {}
    int createSocket() ;
    void Bind(int fd) ;
    int Accept(int fd, char* cliIP, int* cliPort) ;

    //收发数据
    // void sendMsg(std::string &str) override;
    // std::string recvMsg() override;

};



#endif