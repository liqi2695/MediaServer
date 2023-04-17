#include "RTP.h"
#include <string.h>


RtpPacket::RtpPacket() : 
    _mBuf((uint8_t*)malloc(4 + RTP_HEADER_SIZE + RTP_MAX_PKT_SIZE + 100)),
    _mBuf4(_mBuf + 4),
    _rtpHeader((RtpHeader*)_mBuf4),
    _mSize(0) {}

RtpPacket::~RtpPacket() {
    free(_mBuf);
    _mBuf = nullptr;
}



static void parseRtpHeader(uint8_t* buf, RtpHeader* rtpheader) {
    bzero(rtpheader, sizeof(*rtpheader));
    //0位
    rtpheader->version = (buf[0] & 0xC0) >> 6;
    rtpheader->padding = (buf[0] & 0x20) >> 5;
    rtpheader->extension = (buf[0] & 0x10) >> 4;
    rtpheader->csrc = (buf[0] & 0x0F);
    //1位
    rtpheader->mark = (buf[1] & 0x80) >> 7;
    rtpheader->payloadtpye = (buf[1] & 0x7F);
    //2 ~ 3 位
    rtpheader->seqnumber = (buf[2] & 0xFF << 8) | (buf[3] & 0xFF);
    //4 ~ 7位
    rtpheader->timestamp = (buf[4] & 0xFF << 24) |
                           (buf[5] & 0xFF << 16) |
                           (buf[6] & 0xFF << 8)  |
                           (buf[7] & 0xFF);
    //8 ~ 11位
    rtpheader->ssrc = (buf[8] & 0xFF << 24) |
                      (buf[9] & 0xFF << 16) |
                      (buf[10] & 0xFF << 8)  |
                      (buf[11] & 0xFF);
    
}