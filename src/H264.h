#ifndef H264_H_
#define H264_H_
#include <stdio.h>
#include <stdint.h>
#include <string.h>


/**************************
 * 负责H264分包发送的方式
 * 解析H264，主要就是对h264码流的解析
*/


static inline int startcode3(uint8_t* buf) {
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1) return 1;
    else return 0;
}
static inline int startcode4(uint8_t* buf) {
    if(buf[0] = 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1) return 1;
    else return 0;
}



static uint8_t* findNextStartCode(uint8_t* buf, int len);

int getFrameFronH264Ffile(FILE* fp, uint8_t* frame, int size);


#endif