#include<lib/common.h>
#include "inc/process.h"
unsigned int ticks=0;
extern PCB* block_l;
extern PCB* ready_l;
extern int pcb_enqeque(PCB*,PCB*);
extern int pcb_del(PCB*,PCB*);
void do_timer(){
    ticks++;
    cur_pcb->time_lapse++;
    PCB* ptr=block_l;
    while(ptr!=NULL){
        if((--ptr->time_lapse)<=0){
            ptr->ps=READY;
            pcb_del(block_l,ptr);
            pcb_enqeque(ready_l,ptr);
        }
        ptr=ptr->next;
    }
}

unsigned int get_tick(){
    return ticks;
}
