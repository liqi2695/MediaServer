#ifndef ERROR_HELP_H_
#define ERROR_HELP_H_

#include <iostream>
#include <string.h>
#include <errno.h>




static void errMsg() {

}

template <typename T, typename... types> 
static void errMsg(const T& first, const types&... args) {
    
    std::cout <<  stderr << first << strerror(errno) << std::endl;
    errMsg(args...);
}

#endif