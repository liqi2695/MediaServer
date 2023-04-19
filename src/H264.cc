#include "H264.h"




static uint8_t* findNextStartCode(uint8_t* buf, int len) {
    if(len < 3) return nullptr;

    for(int i = 0; i < len - 3; i++) {
        if(startcode3(buf) || startcode4(buf)) return buf;
        ++buf;
    } 
    if(startcode3(buf)) return buf;

    return nullptr;
}


int getFrameFronH264Ffile(File* fp, uint8_t* frame, int size) {

}