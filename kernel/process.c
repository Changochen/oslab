#include "lib/common.h"
#include "inc/string.h"
#include "inc/process.h"
#include "inc/memory.h"
#include "inc/mmu.h"
#include "inc/types.h"
#include "inc/memlayout.h"
#include "inc/pmap.h"
#include "inc/elf.h"
#include "inc/disk.h"
#define ELFADDR 0
#define elf   ((struct ELFHeader *) ELFADDR)


uint32_t entry;
PCB PCBPool[MAXPROCESS];
struct TrapFrame tfPool[MAXPROCESS];
uint32_t pid=0;

PCB* cur_pcb = NULL,*ready_l = NULL,*block_l= NULL;

uint32_t pcb_num(PCB* head){
    uint32_t num = 0;
    PCB* p = head;
    while(p){
        //printf("id:%d\n",p->pid);
        p = p->next;
        num++;
    }
    return num;
}

PCB* pcb_pop(PCB** head){
    PCB* p = *head;
    /*
    *head=(*head)->next;
    p->next=NULL;
    */
    pcb_del(head, p);
    return p;
}


int pcb_enqeque(PCB** head, PCB* p){
    /*
    if(*head == NULL){
        *head = p;
        return 0;
    }else{
        PCB* temp=*head;
        while(temp->next!=NULL){
            temp = temp->next;
        }
        temp->next = p;
        p->next=NULL;
        return 1;
    }
    */
    PCB* h = *head;
    p->next = NULL;
    if(h == NULL){
        *head = p;
        return 0;

    }else{
        while(h->next){
            h = h->next;
        }
        h->next = p;
        return 1;

    }
}
int pcb_del(PCB** head, PCB* p){
    /*
    if(*head==NULL){
        return 0;
    }
    if(*head==p){
        (*head)=(*head)->next;
        p->next=NULL;
    }else{
        PCB* temp=*head;
        while((temp!=NULL)&&(temp->next!=p))temp=temp->next;
        if(temp==NULL)return 0;
        else{
            temp->next=temp->next->next;
            p->next=NULL;
            p->inuse=0;
            return 1;
            }
            }
            return 1;
            */
    PCB *sleep = *head, *pre = NULL;
    while(1){
        if(sleep == NULL)
            return 0;
        if(sleep == p)
            break;
        pre = sleep;
        sleep = sleep->next;
    }
    if(pre == NULL)
        *head = sleep->next;
    else
        pre->next = sleep->next;
    sleep->next = NULL;
    return 1;
}

void pcb_pool_init()
{
    int i;
    for(i=0; i<MAXPROCESS; i++){
        PCBPool[i].inuse = 0;
        //PCBPool[i].next=NULL;
    }
}

void pcb_init(PCB *p, uint32_t ustack, uint32_t entry, uint8_t pri)
{
    struct TrapFrame *tf = (p->tf);
    if(pri == 0){
        tf->eflags = 0x2 | FL_IF;
        tf->ds = tf->es = tf->ss = tf->fs = tf->gs = GD_KD;
        tf->cs = GD_KT;
    }else if(pri == 3){
        /*
        tf->eflags = 0x2 | FL_IF;
        tf->ds = tf->es = tf->ss = tf->fs = tf->gs = GD_KD;
        tf->cs = GD_KT;
        */
        tf->eflags = 0x2 | FL_IF;
        tf->ds = tf->es = tf->ss = tf->fs = tf->gs = GD_UD | 3;
        tf->cs = GD_UT | 3;
    }
    printf("stack:%x,pid %d\n",ustack,p->pid);
    tf->esp = ustack;
    tf->eip = entry;
    if(pri == 0){
        void* ptr1 = (void*)(ustack);
        void* ptr2 = (void*)tf;
        uint32_t siz = sizeof(struct TrapFrame);
        memcpy(ptr1,ptr2,siz);
        p->tf = ptr1;
    }
}

PCB* pcb_create()
{
    uint32_t i=0;
    for(i=0; i<MAXPROCESS; i++){
        if(PCBPool[i].inuse==0)break;
    }
    if(i==MAXPROCESS)return NULL;

    PCB *p = &PCBPool[i];
    p->tf = &tfPool[i];
    p->inuse = 1;

    struct PageInfo *pp = page_alloc(ALLOC_ZERO);
    if (pp == NULL) return NULL;
    p->time_lapse = 0;
    p->pgdir = page2kva(pp);
    printf("Page dir %x\n",(uint32_t)p->pgdir);
    p->pid = pid;
    pid ++;
    pp->pp_ref ++;
    printf("kern_pgdir %x\n",kern_pgdir);
    memcpy(p->pgdir, kern_pgdir, PGSIZE);
    return p;
}

void pcb_ready(PCB* pcb){
    if(ready_l == NULL){
        ready_l = pcb;
    }else{
        PCB* temp = ready_l;
        ready_l = pcb;
        ready_l->next = temp;
    }
}

void pcb_load(PCB* pcb, uint32_t offset){
    struct ProgramHeader *ph, *eph;
    unsigned char* pa, *i;
    lcr3(PADDR(pcb -> pgdir));

    mm_alloc(pcb->pgdir, ELFADDR, 0x1000);
    readseg((unsigned char*)elf, 8*SECTSIZE, offset);

    ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
    eph = ph + elf->phnum;

    for(; ph < eph; ph ++) {
        pa = (unsigned char*)ph->paddr;
        mm_alloc(pcb->pgdir, ph->vaddr, ph->memsz);
        readseg(pa, ph->filesz, offset+ph->off);
        for (i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
    }
    entry = elf->entry;
    printf("Game entry %x\n",entry);
    mm_alloc(pcb->pgdir, USTACKTOP-STACKSIZ, STACKSIZ);
    pcb_init(pcb, USTACKTOP-0x1FF, entry, 3);
    //pcb_init(pcb, USTACKTOP, entry, 0);
    lcr3(PADDR(kern_pgdir));
}

void pcb_funcload(PCB* pcb, void* ptr){
    lcr3(PADDR(pcb -> pgdir));
    entry = (uint32_t)ptr;
    pcb_init(pcb, (uint32_t)pcb->kern_stacktop, entry, 0);
    lcr3(PADDR(kern_pgdir));
}

int schel_num=0;
void schedule(){
    schel_num++;
    while(1){
        if(cur_pcb==NULL){
            cur_pcb=pcb_pop(&ready_l);
            cur_pcb->time_lapse=0;
            cur_pcb->ps=RUNNING;
            /*if(cur_pcb->pid!=0){
            struct TrapFrame *tf = (cur_pcb->tf);
            tf->eflags = 0x2 | FL_IF;
            tf->ds = tf->es = tf->ss = tf->fs = tf->gs = GD_KD;
            tf->cs = GD_KT;
            }
            */
            //printf("ready?%d\n",ready_l==NULL);
            //printf("switch to pid %d,entry %x,tf :%x\n",cur_pcb->pid,cur_pcb->tf->eip,cur_pcb->tf);
            printf("switch to pid %d\n",cur_pcb->pid);
            //printf("eflags:%x,pgdir %x,cs %x\n",cur_pcb->tf->eflags,cur_pcb->pgdir,cur_pcb->tf->cs);
            //cur_pcb->tf->eflags=0x202;
            scheduler_switch(cur_pcb);
            break;
        }else if(cur_pcb->ps==BLOCKED){
            pcb_enqeque(&block_l,cur_pcb);
            cur_pcb=NULL;
        }else if(cur_pcb->time_lapse>1000||cur_pcb->ps==YIELD){
            //            printf("yield\n");
            cur_pcb->ps=READY;
            if(cur_pcb->pid==0){
                printf("hello,again!\n");
            }
            pcb_enqeque(&ready_l,cur_pcb);
            cur_pcb=NULL;
        }else{
            //printf("nothing\n");
            break;
        }
    }
    /*
       if(cur_pcb == NULL){
       cur_pcb = pcb_pop(&ready_l);
       cur_pcb->ps = RUNNING;
       cur_pcb->time_lapse = 0;
       printf("switch to pid %d,\n",cur_pcb->pid);

       scheduler_switch(cur_pcb);
       return;
       }
       if(cur_pcb->ps == BLOCKED){

       pcb_enqeque(&block_l, cur_pcb);
       cur_pcb = NULL;
       schedule();
       return;
       }
       if(cur_pcb->time_lapse > 3 || cur_pcb->ps == YIELD){
       cur_pcb->ps = READY;
       pcb_enqeque(&ready_l, cur_pcb);
       cur_pcb = NULL;
       schedule();
       return;
       }
       */
}

