#ifndef RTP_H_
#define RTP_H_
#include <stdint.h>
#include <stdlib.h>

#define RTP_VESION              2

#define RTP_PAYLOAD_TYPE_H264   96
#define RTP_PAYLOAD_TYPE_AAC    97

#define RTP_HEADER_SIZE         12
#define RTP_MAX_PKT_SIZE        1400

struct RtpHeader {
    uint8_t version : 2;
    uint8_t padding : 1;
    uint8_t extension : 1;
    uint8_t csrc : 4;
    uint8_t mark : 1;
    uint8_t payloadtpye : 7;

    uint16_t seqnumber : 16;
    uint32_t timestamp : 32;
    
    uint32_t ssrc : 32;
};


class RtpPacket {
public:
    RtpPacket();
    ~RtpPacket();
    uint8_t* getMBuf() { return _mBuf; }
    uint8_t* getMbuf4() { return _mBuf4; }

    RtpHeader* getRtpHeader() const { return _rtpHeader; }
    int getMSize() { return _mSize; }
private:
    uint8_t* _mBuf; //4 + rtpHeader + rtpBody
    uint8_t* _mBuf4; //rtpHeader + rtpBody  、
    /*
    * 这里是因为如果RTP,RTCP基于TCP，那么会共用端口，所以需要在报文前加4个字节来辨别
    * 可以看：https://blog.csdn.net/xt18971492243/article/details/123349467
    *  https://blog.csdn.net/weixin_35804181/article/details/87383444
    *  https://www.bilibili.com/video/BV1kG411K7ZX?t=839.6
    *  tempBuf[1] = channel;// 0x00;
    * 其中第2个字节，因为如果用UDP会有RTP，RTCP两个端口，同时又有音视频
    * 所以使用TCP时，这个channel就是 。0~3 0(0x00),1(0x01)表示视频的RTP/RTCP; 2(0x02)，3(0x03)表示音频的RTP/RTCP
    */

    RtpHeader* const _rtpHeader;
    int _mSize;
};


static void parseRtpHeader(uint8_t* buf, RtpHeader* rtpHeader);




#endif