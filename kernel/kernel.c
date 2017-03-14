#include <lib/common.h>

#include <lib/serial.h>
#include <lib/i8259.h>
#include <lib/timer.h>
#include <lib/irq.h>
#include <lib/video.h>
extern uint8_t* VGAP;
//#include "inc/logo.h"
/*
static int letter_code[] = {
	30, 48, 46, 32, 18, 33, 34, 35, 23, 36,
	37, 38, 50, 49, 24, 25, 16, 19, 31, 20,
	22, 47, 17, 45, 21, 44
};
*/
void press(int code){
}
void timer(){
}
int main(){
	init_serial();
	init_timer();
	init_idt();
	init_intr();
	set_keyboard_intr_handler(press);
	set_timer_intr_handler(timer);
	enable_interrupt();
    while(1){
        for(int i=0;i<320*200;i++){
            VGAP[i]=50;
        }
        wait_for_interrupt();
        disable_interrupt();
        enable_interrupt();
    }
    return 1;
};
