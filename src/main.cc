#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "Socket.h"
#include "error_help.h"
#define SERVER_PORT    8080
#define SERVER_RTP_PORT 55534
#define SERVER_RTCP_PORT 55535


//handle方法

static int handleCmd_OPTIONS(char* result, int cseq)
{
    sprintf(result, "RTSP/3.0 200 OK\r\n"
        "CSeq: %d\r\n"
        "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
        "\r\n",
        cseq);

    return 0;
}

static int handleCmd_DESCRIBE(char* result, int cseq, char* url)
{
    char sdp[500];
    char localIp[100];

    sscanf(url, "rtsp://%[^:]:", localIp);

    sprintf(sdp, "v=0\r\n"
        "o=- 9%ld 1 IN IP4 %s\r\n"
        "t=0 0\r\n"
        "a=control:*\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=control:track0\r\n",
        time(NULL), localIp);

    sprintf(result, "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
        "Content-Base: %s\r\n"
        "Content-type: application/sdp\r\n"
        "Content-length: %zu\r\n\r\n"
        "%s",
        cseq,
        url,
        strlen(sdp),
        sdp);

    return 0;
}

static int handleCmd_SETUP(char* result, int cseq, int clientRtpPort)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
        "CSeq: %d\r\n"
        "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
        "Session: 66334873\r\n"
        "\r\n",
        cseq,
        clientRtpPort,
        clientRtpPort + 1,
        SERVER_RTP_PORT,
        SERVER_RTCP_PORT);

    return 0;
}

static int handleCmd_PLAY(char* result, int cseq)
{
    sprintf(result, "RTSP/1.0 200 OK\r\n"
        "CSeq: %d\r\n"
        "Range: npt=0.000-\r\n"
        "Session: 66334873; timeout=10\r\n\r\n",
        cseq);

    return 0;
}


static void doclient(int clifd, const char* cliIP, int cliPort)  
{
    char method[40];
    char url[100];
    char version[40];
    int CSeq;

    int cliRtpPort, cliRtcpPort;
    char* rBuf = (char*)malloc(10000);
    char* sBuf = (char*)malloc(10000);

    while (1)
    {
        int recvlen;
        recvlen = recv(clifd, rBuf, 2000, 0);
        if(recvlen < 0) {
            errMsg("recv error ");
            break;
        }
        /*
         * 打印log信息
        */
        std::cout << "recvlen:" << recvlen << std::endl;
        /*FUNCTION表示当前执行的函数的名称*/
        std::cout << "-------------------------------------------rBuf------------------------------------------------" << std::endl;
        std::cout << __FUNCTION__ << "rBuf = " << rBuf << std::endl;
        std::cout << "-------------------------------------------rBuf------------------------------------------------" << std::endl;

        const char* sep = "\n";
        /*按照\n将rBuf分割*/
        char* line = strtok(rBuf, sep);
        /*下面就是解析rBuf报文*/
        while(line) {
            if(strstr(line, "OPTIONS") ||
               strstr(line, "DESCRIBE") ||
               strstr(line, "SETUP") ||
               strstr(line, "PLAY")) {
                /*如果line中存在某一个，则提取相应字段*/
                if (sscanf(line, "%s %s %s\r\n", method, url, version) != 3) {
                    errMsg("Imcomplete message!");
                }
            } else if (strstr(line, "CSeq")) {
                if(sscanf(line, "CSeq: %d\r\n", &CSeq) != 1) {
                    errMsg("Imcomplete message and no CSeq");
                }
            } else if (!strncmp(line, "Transport:", strlen("Transport:"))) {
                // Transport: RTP/AVP/UDP;unicast;client_port=13358-13359
                // Transport: RTP/AVP;unicast;client_port=13358-13359

                if (sscanf(line, "Transport: RTP/AVP/UDP;unicast;client_port=%d-%d\r\n",
                    &cliRtpPort, &cliRtcpPort) != 2) {
                    
                    errMsg("parse Transport error");
                }
            }
            line = strtok(NULL, sep);
        }

        /*开始根据OPTION方法进行对应handle的方法*/
        if(strcmp(method, "OPTIONS") == 0) {
            if(handleCmd_OPTIONS(sBuf, CSeq)) {
                errMsg("failed to handle options");
                break;
            }
        } else if(strcmp(method, "DESCRIBE") == 0) {
            if(handleCmd_DESCRIBE(sBuf, CSeq, url)) {
                errMsg("failed to handle describe");
                break;
            }
        } else if(strcmp(method, "SETUP") == 0) {
            if(handleCmd_SETUP(sBuf, CSeq, cliRtpPort)) {
                errMsg("failed to handle setup");
                break;
            }
        } else if(strcmp(method, "PLAY") == 0) {
            if(handleCmd_PLAY(sBuf, CSeq)) {
                errMsg("failed to handle play");
                break;
            }
        } else {
            errMsg("undefined method = ", method);
            break;
        }
        std::cout << "-------------------------------------------sBuf------------------------------------------------" << std::endl;
        std::cout << __FUNCTION__ << "sBuf = " << sBuf << std::endl; 
        std::cout << "-------------------------------------------sBuf------------------------------------------------" << std::endl;

        send(clifd, sBuf, strlen(sBuf), 0);
        //开始播放，发送RTP包
        if (!strcmp(method, "PLAY")) {

            printf("start play\n");
            printf("client ip:%s\n", cliIP);
            printf("client port:%d\n", cliRtpPort);

            while (true) {


                sleep(40);
                //usleep(40000);//1000/25 * 1000
            }
   
            break;
        }
        memset(method,0,sizeof(method)/sizeof(char));
        memset(url,0,sizeof(url)/sizeof(char));
        CSeq = 0;
        /* code */
    }
    close(clifd);
    free(rBuf);
    free(sBuf); 
}



int main(int argc, char* argv[]) {
    if(argc <= 2) {
        errMsg("input ip and port");
    }
    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    
    TCPSocket sock(ip, port);
    int servfd = sock.createSocket();

    sock.Bind(servfd);
    listen(servfd, 10);

    while(1) {
        int cliPort;
        char cliIP[100];

        int clitsock = sock.Accept(servfd, cliIP, &cliPort);

        std::cout << "accept client: client IP:" << cliIP << " client PORT: " << cliPort << std::endl;
        doclient(clitsock, ip, cliPort);

    }
    close(servfd);
    return 0;
}
