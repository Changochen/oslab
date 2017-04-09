#include <lib/common.h>

#include <lib/serial.h>
#include <lib/i8259.h>
#include <lib/timer.h>
#include <lib/irq.h>
#include <lib/video.h>

#include "inc/elf.h"
#include "inc/disk.h"
#include "inc/types.h"

extern unsigned int get_tick();
unsigned int  counter=0;
unsigned int counter2=0;
unsigned int counter3=0;

void press(int code){
    switch(code){
        case 30:    //A
            printf("A\n");
            move(LEFT);
            break;
        case 31:    //S
            printf("S\n");
            move(DOWN);
            break;
        case 32:    //D
            printf("D\n");
            move(RIGHT);
            break;
        case 17:    //W
            printf("W\n");
            move(UP);
            break;
    }
}

void timer(){
    if(counter>=200){
        addBlock();
        counter=0;
    }
    if(counter2>=100){
        randomMove();
        counter2=0;
    }
    counter++;
    counter2++;
    counter3++;
    drawFrame();
}
unsigned int my_get_tick(){
    uint32_t t;
    asm volatile("int $0X80": "=a"(t) : "a"(4000));
    return t;
}
int main(){
    printf("game start!\n");
    initVideo(0xFF);
    int flag=1;
    uint32_t old_time=0;
    while(flag){
        unsigned int temp=my_get_tick();
        if(old_time<temp){
            old_time=temp;
            timer();
        }
        if(get_gameState()!=1)flag=0;
    }
    printf("LOSE!You live for %d seconds\n",counter3/100);
    while(1);
    return 1;
};
