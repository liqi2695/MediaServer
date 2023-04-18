#include "AAC.h"
#include <string.h>
#include "error_help.h"

static int parseAdtsHeader(uint8_t* buf, AdtsHeader* adtsHeader) {
    bzero(adtsHeader, sizeof(*adtsHeader));

    //对开始字段进行判断
    if( ((buf[0] & 0xFF) == 0xFF) && ((buf[1] & 0xF0) == 0xF0) ) {
        //1个字节后四位
        adtsHeader->id = (buf[1] & 0x80) >> 3;
        adtsHeader->layer = (buf[1] & 0x60) >> 1;
        adtsHeader->protectionAbsent = (buf[1] & 0x01);

        //第二个字节
        adtsHeader->profile = (buf[2] & 0xC0) >> 6;
        adtsHeader->samplingFrequencyIndex = (buf[2] & 0x3C) >> 2;
        adtsHeader->privateBit = (buf[2] & 0x02) >> 1;
        //3，4个字节
        adtsHeader->channelConfiguration = ((buf[2] & 0x01) << 2) | ((buf[3] & 0xC0) >> 6);
        adtsHeader->orinialCopy = (buf[3] & 0x20) >> 5;
        adtsHeader->home = (buf[3] & 0x10) >> 4;
        adtsHeader->copyrigthIdentificationBit = ((uint8_t)buf[3] & 0x08) >> 3;
        adtsHeader->copyrigthIndectificationStat = (uint8_t)buf[3] & 0x04 >> 2;
        
        adtsHeader->aacFrameLength = ((((buf[3]) & 0x03) << 11) |
            ((buf[4] & 0xFF) << 3) |
            (buf[5] & 0xE0) >> 5);

        adtsHeader->adtsBufferfullness = ((buf[5] & 0x1f) << 6 |
            (buf[6] & 0xfc) >> 2);
        adtsHeader->numberOfRawDataBlocksInFrame = (buf[6] & 0x03);

        return 0;
    } else {
        errMsg("aac format error!");
        return -1;
    }
}