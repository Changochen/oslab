#include<lib/common.h>
unsigned int ticks=0;

void do_timer(){
    ticks++;
}


unsigned int get_tick(){
    return ticks;
}