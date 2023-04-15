#include "RTP.h"
#include <iostream>



void InitRtpHeader(RtpPacket& rtpPacket, uint8_t csrcLen, uint8_t extension, uint8_t padding
    , uint8_t version, uint8_t payloadType, uint8_t marker,
    uint16_t seq, uint32_t timestamp, uint32_t ssrc) 
{
    rtpPacket.rtpHeader.csrcLen = csrcLen;
    rtpPacket.rtpHeader.extension = extension;
    rtpPacket.rtpHeader.padding = padding;
    rtpPacket.rtpHeader.version = version;
    rtpPacket.rtpHeader.payloadType = payloadType;
    rtpPacket.rtpHeader.marker = marker;
    rtpPacket.rtpHeader.seq = seq;
    rtpPacket.rtpHeader.timestamp = timestamp;
    rtpPacket.rtpHeader.ssrc= ssrc;
}


int rtpSendPacketOverUdp(int serverRtpSockfd, const std::string& ip, int16_t port, struct RtpPacket& rtpPacket, uint32_t dataSize) 
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    rtpPacket.rtpHeader.seq = htons(rtpPacket.rtpHeader.seq);
    rtpPacket.rtpHeader.timestamp = htonl(rtpPacket.rtpHeader.timestamp);
    rtpPacket.rtpHeader.ssrc = htonl(rtpPacket.rtpHeader.ssrc); //转网络，再调用sendto

    int ret = sendto(serverRtpSockfd, &rtpPacket, dataSize + RTP_HEADER_SIZE, 0,
        (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0)  { std::cout << "erro send" << std::endl;}
    else {std::cout << "send buf" << std::endl;}

    rtpPacket.rtpHeader.seq = ntohs(rtpPacket.rtpHeader.seq); //转主机
    rtpPacket.rtpHeader.timestamp = ntohl(rtpPacket.rtpHeader.timestamp);
    rtpPacket.rtpHeader.ssrc = ntohl(rtpPacket.rtpHeader.ssrc); 
    
    return ret;
}