#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "inc/memlayout.h"

#define NPUSTACKTOP 0x300000
#define NPKSTACKTOP 0x200000
#define NPKSTACKSIZE 2*4096
#define MAXPROCESS 500
#define STACKSIZ 8192
typedef enum{
    YIELD,
    READY,
    RUNNING,
    BLOCKED,
    WAITING
}PROCESS_STATE;

typedef struct PCB {
    struct{
        PROCESS_STATE ps;
        uint32_t inuse;
        uint32_t pid;
        uint32_t ppid;
        uint32_t time_lapse;
        struct TrapFrame *tf;
        pde_t *pgdir;
    };
    struct PCB *next;
    /*
       struct PCB *tail;
       */
    uint8_t kern_stack[STACKSIZ];
    uint8_t kern_stacktop[16];
    uint8_t user_stack[STACKSIZ];
    uint8_t user_stacktop[16];
    //uint8_t kstackprotect[0x10];
} PCB;

extern PCB *cur_pcb;
extern PCB* ready_l;
extern PCB* block_l;

void pcb_pool_init();
void pcb_init(PCB *p, uint32_t ustack, uint32_t entry, uint8_t privilege);
PCB* pcb_create();
void pcb_enter(PCB*);
void pcb_switch(PCB*);
void pcb_ready(PCB*);

void do_scheduler();

void pcb_load(PCB* pcb, uint32_t offset);
void pcb_funcload(PCB* pcb, void* entry);

uint32_t pcb_num(PCB* head);
PCB*     pcb_pop(PCB* head);
int      pcb_push(PCB* head, PCB* p);
int      pcb_enqeque(PCB* head, PCB* p);
int      pcb_del(PCB* head, PCB* p);
#endif