#include <lib/syscall.h>
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
    printf("sdf\n");
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

char my_key_down(char s){
	char r_eax = 0;
	asm volatile("int $0x80": : "a"(4001), "b"(s));
	asm volatile("movl %%eax, %0\n" : : "m"(r_eax));
	return r_eax;
}
int main(){
    printf("game start!\n");
    initVideo(0xFF);
    int flag=1;
    uint32_t old_time=0;
    while(flag){
        unsigned int temp=system_get_tick();
        if(old_time<temp){
            old_time=temp;
            timer();
        int a=my_key_down('a');
        int s=my_key_down('s');
        int w=my_key_down('w');
        int d=my_key_down('d');
        if(a)press(30);
        else if(s)press(31);
        else if(w)press(17);
        else if(d)press(32);
        }
        if(get_gameState()!=1)flag=0;
    }
    printf("LOSE!You live for %d seconds\n",counter3/100);
    while(1);
    return 1;
};
