#ifndef AAC_H_
#define AAC_H_
#include <stdio.h>
#include <stdint.h>

/*********
 * AAC的音频格式两种：1 ADIF，只有一个统一的头，必须得到所有数据后解码
 * 2 ADTS，音视频数据传输流，每一帧都有头信息，适用传输流
 * 
 * 
 * **********/


struct AdtsHeader {
    //adts头总共占7byte 56bit
    uint16_t synword : 12; /*总是0xFF (1111 1111 1111)，代表ADTS帧的开始*/
    uint8_t id : 1; /*设置MPEG标识符，0标识MPEG-4 1标识MPEG-2*/
    uint8_t layer : 2; /*总是00*/
    uint8_t protectionAbsent : 1; /*校验位，0表示有CRC校验，1表示没有CRC校验*/
    uint8_t profile : 2; /*AAC级别*/

    uint8_t samplingFrequencyIndex : 4; /*采样率下标*/
    uint8_t privateBit : 1; /*编码设为0，解码忽略*/

    uint8_t channelConfiguration : 3;/*声道数*/
    uint8_t orinialCopy : 1; /*编码设为0，解码忽略*/
    uint8_t home : 1; /*编码设为0，解码忽略*/
    uint8_t copyrigthIdentificationBit : 1; /*编码设为0，解码忽略*/
    uint8_t copyrigthIndectificationStat : 1; /*编码设为0，解码忽略*/

    uint16_t aacFrameLength : 13; /*ADTS帧长度，包括ADTS头和AAC原始流*/
    uint16_t adtsBufferfullness : 11; /*缓冲区充满度*/
    uint8_t numberOfRawDataBlocksInFrame : 2; /*表示ADTS*/

};

static int parseAdtsHeader(uint8_t* buf, AdtsHeader* astsHeader);

#endif