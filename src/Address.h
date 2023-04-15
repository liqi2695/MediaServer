#ifndef ADDRESS_H_
#define ADDRESS_H_

#include "Socket.h"

//对传过来得字符串进行一个完美转换得意思，string or char[] 都接收
class String {
public:
    String(const char* str) : _str(str) {}

    String(const std::string& str) : _str(str.c_str()) {}

    const char* get_str() const { return  _str; }

private:
    const char* _str;
};




class InetAddress {
public:
    //构造函数
    explicit InetAddress(uint16_t port = 0);
    InetAddress(String ip, uint16_t port);

    //
    InetAddress(const struct sockaddr_in& addr) : _addr(addr) {}

    struct sockaddr_in getSockAddr() const {return _addr; }

    sa_family_t family() const {return _addr.sin_family; }

    uint32_t ipv4NetEndian() const { return _addr.sin_addr.s_addr; }


private:
    struct sockaddr_in _addr;

};







#endif