#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "inc/memlayout.h"

#define NPKSTACKSIZE 2*4096
#define MAXPROCESS 500
#define STACKSIZ 8192*3
#define SYSTEM_PRI 0
#define USER_PRI   3
typedef enum{
    YIELD,
    READY,
    RUNNING,
    BLOCKED,
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
    uint8_t kern_stack[STACKSIZ];
    uint8_t kern_stacktop[16];
    uint8_t user_stack[USTACKSIZE];
    uint8_t user_stacktop[16];
    //uint8_t kstackprotect[0x10];
} PCB;

typedef struct {
    int                     lock;
    unsigned int            count;
    PCB*                    wait_list;

}Sem;

extern PCB *cur_pcb;
extern PCB* ready_l;
extern PCB* block_l;

void pcb_pool_init();
void pcb_init(PCB *p, uint32_t ustack, uint32_t entry, uint8_t privilege);
PCB* pcb_create();
void pcb_enter(PCB*);
void pcb_ready(PCB*);

void schedule();

void pcb_load(PCB* pcb, uint32_t offset);
void pcb_funcload(PCB* pcb, void* entry,int priv);
void scheduler_switch(PCB*);
uint32_t pcb_num(PCB* head);
PCB*     pcb_pop(PCB** head);
int      pcb_enqeque(PCB** head, PCB* p);
int      pcb_del(PCB** head, PCB* p);

int fork();
void sem_init(Sem* sem,int count);
void sem_destroy(Sem* sem);
void sem_post(Sem* sem);
void sem_wait(Sem* sem);
int sem_trywait(Sem* sem);
#endif
