#include <lib/common.h>

#include <lib/serial.h>
#include <lib/i8259.h>
#include <lib/timer.h>
#include <lib/irq.h>
#include <lib/video.h>

int counter=0;
int counter2=0;
void press(int code){
    switch(code){
        case 30:    //A
            printk("A\n");
            move(LEFT);
            break;
        case 31:    //S
            printk("S\n");
            move(DOWN);
            break;
        case 32:    //D
            printk("D\n");
            move(RIGHT);
            break;
        case 17:    //W
            printk("W\n");
            move(UP);
            break;
    }
}
void timer(){
    if(counter==500){
        addBlock();
        counter=0;
    }
    if(counter2==50){
        randomMove();
        counter2=0;
    }
    counter++;
    counter2++;
    drawFrame();
}
int main(){
	init_serial();
	init_timer();
	init_idt();
	init_intr();
	set_keyboard_intr_handler(press);
	set_timer_intr_handler(timer);
	enable_interrupt();
    initVideo(0xFF);
    while(1){
        wait_for_interrupt();
        disable_interrupt();
        enable_interrupt();
    }
    return 1;
};
