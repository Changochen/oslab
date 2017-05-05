#include <lib/common.h>
#include <lib/serial.h>
#include <lib/i8259.h>
#include <lib/timer.h>
#include <lib/irq.h>
#include <lib/video.h>


#include "inc/elf.h"
#include "inc/disk.h"
#include "inc/types.h"
#include "inc/pmap.h"
#include "inc/process.h"
#include "lib/syscall.h"
#define elf   ((struct ELFHeader *) 102400)
#define ELF 102400
extern void init_segment();
void nothing(){}
void nothing2(int k){}

void init(){
  while(1){
    system_yield();
  }
}

int main(){
    init_page();
    init_segment();
    init_serial();
    init_timer();
    init_idt();
    init_intr();
    set_keyboard_intr_handler(nothing2);
    pcb_pool_init();

    PCB *pidle = pcb_create();
    pcb_funcload(pidle, init);
    PCB *pcb = pcb_create();
    PCB *pcb2 = pcb_create();
    pcb_load(pcb, 102400);
    pcb_load(pcb2, 102400);
    pcb_ready(pidle);
    pcb_ready(pcb);
    pcb_ready(pcb2);
    schedule();
    printf("%s\n", "Never return otherwise you are fucked!");
    while (1){};
    return 1;
};
