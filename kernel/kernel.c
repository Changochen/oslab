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

void idle(){
  while(1){
    system_yield();
  }
}


void busy(){
    uint32_t tick=0;
    while(1){
        //``printf("This is busy! Pid %ds! Recall %d times\n", system_getpid(), tick);
        tick++;
        system_yield();
        system_sleep(system_getpid()*200);
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

    printf("new beginning\n");
    PCB *pidle = pcb_create();
    pcb_funcload(pidle, idle);
    //PCB *pcb = pcb_create();
    //PCB *pcb2 = pcb_create();
    //pcb_load(pcb, 102400);
    //pcb_load(pcb2, 102400);
    pcb_ready(pidle);
    //pcb_ready(pcb);
   // pcb_ready(pcb2);
    uint32_t i = 0;
    for (i = 0; i < 10; i++)
    {
        PCB *pcc = pcb_create();
        pcb_funcload(pcc, busy);
        pcb_ready(pcc);
    }
    schedule();
    /*
       struct ProgramHeader *ph, *eph;
       unsigned char *pa, *i;

       readseg((unsigned char *)elf, 8 * SECTSIZE, 102400);

       printf("Magic Assertion: %x\n", (elf->magic == 0x464C457FU));

       ph = (struct ProgramHeader *)((char *)elf + elf->phoff);
       eph = ph + elf->phnum;
       for (; ph < eph; ph++)
       {
       pa = (unsigned char *)ph->paddr;
       readseg(pa, ph->filesz, 102400 + ph->off);
       for (i = pa + ph->filesz; i < pa + ph->memsz; *i++ = 0)
       ;
       }
       enable_interrupt();
       ((void (*)(void))elf->entry)();
       */
    printf("%s\n", "Never return otherwise you are fucked!");
    while (1)
    {
    }
    return 1;
};
