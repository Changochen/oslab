#include<lib/video.h>
#include<inc/string.h>

uint8_t Frame[320*200];
uint8_t* VGAP=(uint8_t*)0xa0000;
void drawFrame(){
    memcpy(VGAP,Frame,VGA_SIZ);
}


