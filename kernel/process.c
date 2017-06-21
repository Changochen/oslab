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
#include "inc/fs.h"
#define ELFADDR 0
#define elf   ((struct ELFHeader *) ELFADDR)
#define PSZIE 0x1000
#define USER_STACK 0xeebfe000
#define MDEBUG 0
#define INODE_NUM 32
#define BLOCK_OFFSET 98
#define BITMAP_OFFSET 0
#define BITMAP_SIZE (64*SECTSIZE)
#define DIR_OFFSET (64*SECTSIZE)
#define DIR_SIZE (2*SECTSIZE)
#define INODE_OFFSET (66*SECTSIZE)
#define INODE_SIZE (32*SECTSIZE)
#define MAXOPENFILE 256
#define block_to_address(A) ((A+BLOCK_OFFSET)*SECTSIZE)
#define offset_to_block_offset(A)  ((A/512))
#define offset_left(A)      (512-(A%512))
uint32_t entry;
PCB PCBPool[MAXPROCESS];
FCB FCBPool[MAXOPENFILE];
struct TrapFrame tfPool[MAXPROCESS];
uint32_t pid=0;
PCB* cur_pcb = NULL,*ready_l = NULL,*block_l= NULL;

dir directory;
inode inodes[INODE_NUM];
bitmap bmap;

unsigned char bits[]={
    0x80,0x40,0x20,0x10,8,4,2,1
};

const char banner_c[7][208]={
"   #    ######   #####  ######  ####### #######  #####  #     #   ###         # #    #  #       #     # #     # ####### ######   #####  ######   #####  ####### #     # #     # #     # #     # #     # ####### ",
"  # #   #     # #     # #     # #       #       #     # #     #    #          # #   #   #       ##   ## ##    # #     # #     # #     # #     # #     #    #    #     # #     # #  #  #  #   #   #   #       #  ",
" #   #  #     # #       #     # #       #       #       #     #    #          # #  #    #       # # # # # #   # #     # #     # #     # #     # #          #    #     # #     # #  #  #   # #     # #       #   ",
"#     # ######  #       #     # #####   #####   #  #### #######    #          # ###     #       #  #  # #  #  # #     # ######  #     # ######   #####     #    #     # #     # #  #  #    #       #       #    ",
"####### #     # #       #     # #       #       #     # #     #    #    #     # #  #    #       #     # #   # # #     # #       #   # # #   #         #    #    #     #  #   #  #  #  #   # #      #      #     ",
"#     # #     # #     # #     # #       #       #     # #     #    #    #     # #   #   #       #     # #    ## #     # #       #    #  #    #  #     #    #    #     #   # #   #  #  #  #   #     #     #      ",
"#     # ######   #####  ######  ####### #        #####  #     #   ###    #####  #    #  ####### #     # #     # ####### #        #### # #     #  #####     #     #####     #     ## ##  #     #    #    ####### "
};
int get_dir_entry_by_name(const char* pathname);
unsigned int find_new_block();
uint32_t pcb_num(PCB* head){
    uint32_t num = 0;
    PCB* p = head;
    while(p){
        p = p->next;
        num++;
    }
    return num;
}

PCB* pcb_pop(PCB** head){
    PCB* p = *head;
    *head=(*head)->next;
    p->next=NULL;
    return p;
}


int pcb_enqeque(PCB** head, PCB* p){
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
}

int fcb_enqeque(FCB** head, FCB* p){
    if(*head == NULL){
        *head = p;
        return 0;
    }else{
        FCB* temp=*head;
        while(temp->next!=NULL){
            temp = temp->next;
        }
        temp->next = p;
        p->next=NULL;
        return 1;
    }
}

int pcb_del(PCB** head, PCB* p){
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
}

int fcb_del(FCB** head, FCB* p){
    if(*head==NULL){
        return -1;
    }
    if(*head==p){
        (*head)=(*head)->next;
        p->next=NULL;
        p->inuse=0;
    }else{
        FCB* temp=*head;
        while((temp!=NULL)&&(temp->next!=p))temp=temp->next;
        if(temp==NULL)return -1;
        else{
            temp->next=temp->next->next;
            p->next=NULL;
            p->inuse=0;
            return 1;
        }
    }
    return 1;
}

void pcb_pool_init()
{
    int i;
    for(i=0; i<MAXPROCESS; i++){
        PCBPool[i].inuse = 0;
        PCBPool[i].files=NULL;
    }
}

void fcb_pool_init(){
    int i;
    for(i=0; i<MAXOPENFILE; i++){
        FCBPool[i].inuse = 0;
        FCBPool[i].next=NULL;
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
        tf->eflags = 0x2 | FL_IF;
        tf->ds = tf->es = tf->ss = tf->fs = tf->gs = GD_UD | 3;
        tf->cs = GD_UT | 3;
    }
    if(MDEBUG)printf("stack:%x,pid %d\n",ustack,p->pid);
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
    p->open_file_num=0;

    struct PageInfo *pp = page_alloc(ALLOC_ZERO);
    if (pp == NULL) return NULL;
    p->time_lapse = 0;
    p->pgdir = page2kva(pp);
    p->pid = pid;
    pid ++;
    pp->pp_ref ++;
    memcpy(p->pgdir, kern_pgdir, PGSIZE);
    return p;
}

FCB* fcb_create(){
    uint32_t i=0;
    for(i=0; i<MAXOPENFILE; i++){
        if(FCBPool[i].inuse==0)break;
    }
    if(i==MAXOPENFILE)return NULL;
    FCB *p = &FCBPool[i];
    p->inuse=1;
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

    mm_alloc(pcb->pgdir, ELFADDR, PGSIZE);
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
    mm_alloc(pcb->pgdir, USTACKTOP-STACKSIZ, STACKSIZ);
    pcb_init(pcb, USTACKTOP-0x1FF, entry, 3);
    lcr3(PADDR(kern_pgdir));
}

void program_load(PCB* pcb,const char* prog){
    int off=get_dir_entry_by_name(prog);
    unsigned int offset=inodes[off].blocks[0];
    offset=block_to_address(offset);
    pcb_load(pcb,offset);
}

void pcb_funcload(PCB* pcb, void* ptr,int pri){
    lcr3(PADDR(pcb -> pgdir));
    entry = (uint32_t)ptr;
    pcb_init(pcb, (uint32_t)pcb->kern_stacktop, entry, pri);
    lcr3(PADDR(kern_pgdir));
}

void schedule(){
    while(1){
        if(cur_pcb==NULL){
            cur_pcb=pcb_pop(&ready_l);
            cur_pcb->time_lapse=0;
            cur_pcb->ps=RUNNING;
            if(MDEBUG)printf("switch to pid %d\n",cur_pcb->pid);
            scheduler_switch(cur_pcb);
            break;
        }else if(cur_pcb->ps==BLOCKED){
            pcb_enqeque(&block_l,cur_pcb);
            cur_pcb=NULL;
        }else if(cur_pcb->time_lapse>5||cur_pcb->ps==YIELD){
            cur_pcb->ps=READY;
            pcb_enqeque(&ready_l,cur_pcb);
            cur_pcb=NULL;
        }else{
            break;
        }
    }
}

int fork(){
    PCB* fork_pcb=pcb_create();
    if(fork_pcb==NULL){
        return -1;
    }
    int old_pid=fork_pcb->pid;
    pte_t* old_pte=fork_pcb->pgdir;
    cur_pcb->tf->eax=old_pid;
    memcpy((void*)fork_pcb,(void*)cur_pcb,sizeof(PCB));
    fork_pcb->pgdir=old_pte;
    fork_pgdir(fork_pcb->pgdir,cur_pcb->pgdir);
    fork_pcb->ppid=cur_pcb->pid;
    fork_pcb->pid=old_pid;
    fork_pcb->tf=(struct TrapFrame*)((uint32_t)fork_pcb->kern_stack+((uint32_t)cur_pcb->tf-(uint32_t)cur_pcb->kern_stack));
    fork_pcb->tf->eax=0;
    pcb_ready(fork_pcb);
    return 0;
}

int thread_create(uint32_t func){
    PCB* thread_pcb=pcb_create();
    if(thread_pcb==NULL){
        return -1;
    }
    int old_pid=thread_pcb->pid;
    pte_t* old_pte=thread_pcb->pgdir;
    memcpy((void*)thread_pcb,(void*)cur_pcb,sizeof(PCB));
    thread_pcb->pgdir=old_pte;
    thread_pcb->pid=old_pid;
    memcpy(thread_pcb->pgdir,cur_pcb->pgdir,PGSIZE);
    thread_pcb->ppid=cur_pcb->pid;
    mm_alloc(thread_pcb->pgdir,USER_STACK-2*STACKSIZ,2*STACKSIZ);
    thread_pcb->tf=(struct TrapFrame*)((uint32_t)thread_pcb->kern_stack+((uint32_t)cur_pcb->tf-(uint32_t)cur_pcb->kern_stack));
    thread_pcb->tf->esp=USER_STACK-0x80;
    thread_pcb->tf->eip=func;
    pcb_ready(thread_pcb);
    return 0;
}

void sem_init(Sem* sem,int count){
    asm volatile("cli");
    sem->count=count;
    sem->wait_list=NULL;
    asm volatile("sti");
}

void sem_destroy(Sem* sem){
    asm volatile("cli");
    sem=0;
    asm volatile("sti");
}

void sem_post(Sem* sem){
    asm volatile("cli");
    if(sem->wait_list==NULL){
        if(MDEBUG)printf("Simple Post\n");
        sem->count++;
        asm volatile("sti");
    }
    else{
        if(MDEBUG)printf("Post to wait_list\n");
        PCB* temp=pcb_pop(&sem->wait_list);
        pcb_ready(temp);
        cur_pcb->ps=YIELD;
        asm volatile("sti");
        schedule();
    }
}

void sem_wait(Sem* sem){
    if(sem->count==0){
        pcb_enqeque(&sem->wait_list,cur_pcb);
        cur_pcb=NULL;
        schedule();
    }
    asm volatile("cli");
    sem->count--;
    asm volatile("sti");
}

int sem_trywait(Sem* sem){
    int res=1;
    asm volatile("cli");
    if(sem->count==0)res=-1;
    else sem->count--;
    asm volatile("sti");
    return res;
}

void init_fs(){
    readseg(bmap.mask,BITMAP_SIZE,BITMAP_OFFSET);
    readseg((unsigned char *)directory.entries,DIR_SIZE,DIR_OFFSET);
    readseg((unsigned char*)inodes,INODE_SIZE,INODE_OFFSET);
    fcb_pool_init();
}

int get_dir_entry_by_name(const char* pathname){
    for(int i=0;i!=INODE_NUM;i++){
        if(strcmp(pathname,directory.entries[i].filename)==0)return i;
    }
    return -1;
}

void start_game(){
    PCB *pcb = pcb_create();
    program_load(pcb,"game.bin");
    pcb_ready(pcb);
}
void ls(){
    for(int i=0;i!=INODE_NUM;i++){
        if(directory.entries[i].file_size!=0){
            printf("%s\n",directory.entries[i].filename);
        }
    }
}
FCB* get_fcb_by_fd(int fd){
    FCB* res=cur_pcb->files;
    while(res!=NULL){
        if(res->fd==fd)return res;
        res=res->next;
    }
    return NULL;
}

unsigned int get_new_entry(){
    for(int i=0;i!=INODE_NUM;i++){
        if(strlen(directory.entries[i].filename)==0)return i;
    }
    return 0;
}

int open(const char *pathname, int flags){
    int off=get_dir_entry_by_name(pathname);
    if(off==-1){
        if((flags&O_CREAT)==0)return -1;
        off=get_new_entry();
        if(off==0)return -1;
        memcpy(directory.entries[off].filename,pathname,strlen(pathname));
        directory.entries[off].inode_offset=off;
        directory.entries[off].file_size=0;
        inodes[off].blocks[0]=find_new_block();
    }
    FCB* p=fcb_create();
    p->inode_offset=off;
    p->flag=flags;
    p->fd=(cur_pcb->open_file_num++)+3;
    p->offset=0;
    p->next=NULL;
    fcb_enqeque(&cur_pcb->files,p);
    return p->fd;
}


unsigned int find_new_block(){
    unsigned int res;
    for(unsigned int i=0; i<(SECTSIZE* 64);i++){
        if(bmap.mask[i]==0xFF)continue;
        for(int j=0;j!=8;j++){
            if((bmap.mask[i]&bits[j])==0){
                bmap.mask[i]|=bits[j];
                res=i*8+j;
                break;
            }
        }
        return res;
    }
    return 0;
}

unsigned int big_file_block_offset(unsigned int block_offset,unsigned int next_block_num){
    unsigned int paddr=block_to_address(next_block_num);
    inode tmp;
    readseg((unsigned char*)tmp.blocks,SECTSIZE,paddr);
    if(block_offset<511){
        return tmp.blocks[block_offset];
    }else{
        return big_file_block_offset(block_offset-511,tmp.blocks[511]);
    }
}

int read(int fd, void *buf, int len){
    FCB* file=get_fcb_by_fd(fd);
    if((file->flag&O_RDONLY)==0)return -1;
    if((file->offset+len)>directory.entries[file->inode_offset].file_size){
        len=directory.entries[file->inode_offset].file_size-file->offset;
        if(len<=0)return -1;
    }
    unsigned int cur_sec=offset_left(file->offset);
    unsigned block_offset=offset_to_block_offset(file->offset);
    unsigned int paddr=0;
    if(block_offset<127){
        paddr=block_to_address(inodes[file->inode_offset].blocks[block_offset]);
    }else{
        paddr=block_to_address(big_file_block_offset(block_offset-127,inodes[file->inode_offset].blocks[127]));
    }
    paddr+=(512-cur_sec);
    unsigned char tmp[512];
    if(cur_sec>len){
        memset(tmp,0,SECTSIZE);
        readseg(tmp,SECTSIZE,paddr);
        memcpy(buf,tmp,len);
        file->offset+=len;
        return len;
    }else{
        memset(tmp,0,SECTSIZE);
        readseg(tmp,SECTSIZE,paddr);
        memcpy(buf,tmp,cur_sec);
        file->offset+=cur_sec;
        return cur_sec+read(fd,buf+cur_sec,len-cur_sec);
    }
}

int write(int fd, void *buf, int len){
    FCB* file=get_fcb_by_fd(fd);
    if((file->flag&O_WRONLY)==0)return -1;
    unsigned int cur_sec=offset_left(file->offset);
    unsigned block_offset=offset_to_block_offset(file->offset);
    unsigned block=0;
    if(block_offset<127){
        block=inodes[file->inode_offset].blocks[block_offset];
    }else{
        block=big_file_block_offset(block_offset-127,inodes[file->inode_offset].blocks[127]);
    }
    if(block==0){
        inodes[file->inode_offset].blocks[block_offset]=block=find_new_block();
    }
    unsigned int paddr=block_to_address(block);
    paddr+=(SECTSIZE-cur_sec);
    unsigned char tmp[512];
    if(cur_sec>len){
        memset(tmp,0,SECTSIZE);
        readseg(tmp,SECTSIZE,paddr);
        memcpy(tmp+(SECTSIZE-cur_sec),buf,len);
        writeseg(tmp,SECTSIZE,paddr);
        file->offset+=len;
        if(file->offset>directory.entries[file->inode_offset].file_size){
            directory.entries[file->inode_offset].file_size=file->offset;
        }
        writeseg(bmap.mask,BITMAP_SIZE,BITMAP_OFFSET);
        writeseg((unsigned char *)directory.entries,DIR_SIZE,DIR_OFFSET);
        writeseg((unsigned char*)inodes,INODE_SIZE,INODE_OFFSET);
        return len;
    }else{
        memset(tmp,0,SECTSIZE);
        readseg(tmp,SECTSIZE,paddr);
        memcpy(tmp+(SECTSIZE-cur_sec),buf,cur_sec);
        writeseg(tmp,SECTSIZE,paddr);
        file->offset+=cur_sec;
        if(file->offset>directory.entries[file->inode_offset].file_size){
            directory.entries[file->inode_offset].file_size=file->offset;
        }
        return cur_sec+write(fd,buf+cur_sec,len-cur_sec);
    }
    return 1;
}

int lseek(int fd, int offset, int whence){
    FCB* tmp=get_fcb_by_fd(fd);
    switch((OFFSET_STATE)whence){
        case SEEK_SET:
            tmp->offset=offset;
            break;
        case SEEK_CUR:
            tmp->offset+=offset;
            break;
        case SEEK_END:
            tmp->offset=directory.entries[tmp->inode_offset].file_size;
            tmp->offset-=offset;
            break;
    }
    return tmp->offset;
}

int close(int fd){
    FCB* file=get_fcb_by_fd(fd);
    return fcb_del(&cur_pcb->files,file);
}


void cat(char* filename){
    int fd=open(filename,O_RDONLY);
    int siz;
    char buffer[11];
    while((siz=read(fd,buffer,10))>0){
        buffer[siz]='\0';
        printf("%s",buffer);
    }
}

void mv(char* srcfile,char* destfile){
    int i=get_dir_entry_by_name(srcfile);
    if(i==-1)return;
    memset(directory.entries[i].filename,0,24);
    memcpy(directory.entries[i].filename,destfile,strlen(destfile));
}

void rm(char* filename){
    int i=get_dir_entry_by_name(filename);
    if(i==-1)return;
    memset(directory.entries[i].filename,0,24);
    directory.entries[i].file_size=0;
}

void cp(char* srcfile,char* destfile){
    if(strcmp(srcfile,destfile)==0)return;
    int i=get_dir_entry_by_name(srcfile);
    if(i==-1)return;
    unsigned int newi=get_new_entry();
    if(newi==0)return;
    memcpy(directory.entries[newi].filename,destfile,strlen(destfile));
    directory.entries[newi].file_size=directory.entries[i].file_size;
    directory.entries[newi].inode_offset=newi;
    unsigned char buffer[512];
    for(int j=0;j!=128;j++){
        if(inodes[i].blocks[j]==0)return;
        inodes[newi].blocks[j]=find_new_block();
        unsigned int paddr=block_to_address(inodes[i].blocks[j]);
        readseg(buffer,SECTSIZE,paddr);
        paddr=block_to_address(inodes[newi].blocks[j]);
        writeseg(buffer,SECTSIZE,paddr);
    }
}

void banner(char* bp){
    int len=strlen(bp);
    if(len>10)len=10;
    char buffer[7][80];
    memset(buffer,0,7*80);
    for(int i=0;i<len;i++){
        if(bp[i]>64&&bp[i]<91)continue;
        if(bp[i]>96&&bp[i]<123)bp[i]-=32;
        else{
            printf("Unsupported character!\n");
            return;
        }
    }
    for(int i=0;i<len;i++){
        for(int j=0;j<7;j++){
            memcpy(buffer[j]+i*8,banner_c[j]+8*(bp[i]-65),8);
        }
    }
    for(int i=0;i<7;i++){
        buffer[6][79]=0;
        printf("%s\n",buffer[i]);
    }
}