#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


#define USER_LIMIT 5
#define BUFFER_SIZE 1024 
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBRT 1024
#define PROCESS_LIMIT 65535

struct client_data {
    sockaddr_in address;
    int connfd;
    pid_t pid;                 /* 处理连接此子进程的PID */
    int pipefd[2];             /* 和父进程通信用的管道 */
};


