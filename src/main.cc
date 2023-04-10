#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "Socket.h"
#include "error_help.h"
#include "RTP.h"
#define SERVER_PORT    8080
#define SERVER_RTP_PORT 55534
#define SERVER_RTCP_PORT 55535

#define H264_FILE_NAME   "../data/test.h264"


static int createUdpSocket()
{
    int sockfd;
    int on = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        return -1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    return sockfd;
}

static int bindSocketAddr(int sockfd, const char* ip, int port)
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}


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


/***********************
 * 开始RTP包的封装解封装部分
 * 
*/

static inline int startCode3(char* buf) {
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1) 
        return 1;
    else 
        return 0;
}

static inline int startCode4(char* buf) {
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1) 
        return 1;
    else 
        return 0;
}

static char* findNextStartCode(char* buf, int len) {
    int i;
    if(len < 3) return NULL;
    for(i = 0; i < len - 3; ++i) {
        if(startCode3(buf) || startCode4(buf)) return buf;

        ++buf;
    }
    if(startCode3(buf)) return buf;
    return NULL;
}

static int getFrameFromH264File(FILE* fp, char* frame, int size) {
    if(fp < 0) return -1;

    int rSize, frameSize;
    char* nextStartCode;

    rSize = fread(frame, 1, size, fp);

    if(!startCode3(frame) && !startCode4(frame)) return -1;

    nextStartCode = findNextStartCode(frame+3, rSize-3);
    if(!nextStartCode) {
        return -1;
    } else {
        frameSize = nextStartCode - frame;
        fseek(fp, frameSize - rSize, SEEK_CUR);
    }
    return frameSize;
}

static int rtpSendH264Frame(int servfd, std::string ip, uint8_t port,
    struct RtpPacket& rtpPacket, char* frame, uint32_t frameSize) {
        uint8_t naluType;
        int sendBytes = 0;
        int ret;

        naluType = frame[0];

        std::cout << "frameSize = " << frameSize << std::endl;

   if (frameSize <= RTP_MAX_PKT_SIZE) // nalu长度小于最大包长：单一NALU单元模式
    {

         //*   0 1 2 3 4 5 6 7 8 9
         //*  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         //*  |F|NRI|  Type   | a single NAL unit ... |
         //*  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        memcpy(rtpPacket.payload, frame, frameSize);
        ret = rtpSendPacketOverUdp(servfd, ip, port, rtpPacket, frameSize);
        if(ret < 0)
            return -1;

        rtpPacket.rtpHeader.seq++;
        sendBytes += ret;
        if ((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8) // 如果是SPS、PPS就不需要加时间戳
            goto out;
    }
    else // nalu长度小于最大包场：分片模式
    {

         //*  0                   1                   2
         //*  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
         //* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         //* | FU indicator  |   FU header   |   FU payload   ...  |
         //* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+



         //*     FU Indicator
         //*    0 1 2 3 4 5 6 7
         //*   +-+-+-+-+-+-+-+-+
         //*   |F|NRI|  Type   |
         //*   +---------------+



         //*      FU Header
         //*    0 1 2 3 4 5 6 7
         //*   +-+-+-+-+-+-+-+-+
         //*   |S|E|R|  Type   |
         //*   +---------------+


        int pktNum = frameSize / RTP_MAX_PKT_SIZE;       // 有几个完整的包
        int remainPktSize = frameSize % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
        int i, pos = 1;

        // 发送完整的包
        for (i = 0; i < pktNum; i++)
        {
            rtpPacket.payload[0] = (naluType & 0x60) | 28;
            rtpPacket.payload[1] = naluType & 0x1F;

            if (i == 0) //第一包数据
                rtpPacket.payload[1] |= 0x80; // start
            else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
                rtpPacket.payload[1] |= 0x40; // end

            memcpy(rtpPacket.payload+2, frame+pos, RTP_MAX_PKT_SIZE);
            ret = rtpSendPacketOverUdp(servfd, ip, port, rtpPacket, RTP_MAX_PKT_SIZE+2);
            if(ret < 0)
                return -1;

            rtpPacket.rtpHeader.seq++;
            sendBytes += ret;
            pos += RTP_MAX_PKT_SIZE;
        }

        // 发送剩余的数据
        if (remainPktSize > 0)
        {
            rtpPacket.payload[0] = (naluType & 0x60) | 28;
            rtpPacket.payload[1] = naluType & 0x1F;
            rtpPacket.payload[1] |= 0x40; //end

            memcpy(rtpPacket.payload+2, frame+pos, remainPktSize+2);
            ret = rtpSendPacketOverUdp(servfd, ip, port, rtpPacket, remainPktSize+2);
            if(ret < 0)
                return -1;

            rtpPacket.rtpHeader.seq++;
            sendBytes += ret;
        }
    }
    rtpPacket.rtpHeader.timestamp += 90000 / 25;
    out:

    return sendBytes;
         

}


static void doclient(int clifd, const char* cliIP, int cliPort)  
{
    char method[40];
    char url[100];
    char version[40];
    int CSeq;

    int servRtpSockfd = -1, servRtcpSockfd = -1;
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

            //UDP部分还得写，直接裸API写
            servRtpSockfd = createUdpSocket();
            servRtcpSockfd = createUdpSocket();

            if(servRtpSockfd < 0 || servRtcpSockfd < 0) {
                errMsg("failed to create udp socket");
                break;
            }
            if(bindSocketAddr(servRtpSockfd, "0.0.0.0", SERVER_RTP_PORT) < 0 ||
                bindSocketAddr(servRtcpSockfd, "0.0.0.0", SERVER_RTCP_PORT)) {
                errMsg("failed to bind addr");
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

            int frameSize, startCode;
            char* frame = (char*)malloc(500000);
            struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(500000);
            
            FILE* fp = fopen(H264_FILE_NAME, "rb");
            if (!fp) {
                printf("读取 %s 失败\n", H264_FILE_NAME);
                break;
            }
            InitRtpHeader(*rtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264, 0,
                0, 0, 0x88923423);

            printf("start play\n");
            printf("client ip:%s\n", cliIP);
            printf("client port:%d\n", cliRtpPort);

            while (true) {
                frameSize = getFrameFromH264File(fp, frame, 500000);
                if (frameSize < 0)
                {
                    printf("读取%s结束,frameSize=%d \n", H264_FILE_NAME, frameSize);
                    break;
                }

                if (startCode3(frame))
                    startCode = 3;
                else
                    startCode = 4;

                frameSize -= startCode;
                rtpSendH264Frame(servRtpSockfd, cliIP, cliRtpPort,
                    *rtpPacket, frame + startCode, frameSize);

               


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
