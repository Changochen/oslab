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
#include "inc/process.h"
extern unsigned int get_tick();
unsigned int  counter=0;
unsigned int counter2=0;
unsigned int counter3=0;
Sem mutex;
int products[100]={0};
int product_c=0;
int product_id=123542;
void press(int code){
    switch(code){
        case 30:
            move(LEFT);
            break;
        case 31:
            move(DOWN);
            break;
        case 32:
            move(RIGHT);
            break;
        case 17:
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
    system_flash_screen();
    drawFrame();
}

void producer(){
    printk("Producer thread!\n");
    int c=1;
    while(1){
        system_sem_wait(&mutex);
        product_id=product_id*3179%123546;
        products[product_c++]=product_id;
        printk("Produce success!,product id %d\n",product_id);
        if(c++%(product_id%5+1)==0)system_yield();
        system_sem_post(&mutex);
    }
}

void consumer(){
    printk("Consumer thread!\n");
    int c=1;
    while(1){
        system_sem_wait(&mutex);
        if(product_c!=0){
        printk("Get the %d product!They are:\n",product_c);
        for(int i=0;i<product_c;i++){
            printk("%d\n",products[i]);
        }
            product_c=0;
        }
        if(c++%(product_id%5+1)==0)system_yield();
        system_sem_post(&mutex);
    }
}

int main(){
    printk("game start!\n");
    system_sem_init(&mutex,1);
    system_thread_create((uint32_t)producer);
    system_thread_create((uint32_t)consumer);
    /*
    int pid=system_fork();
    if(pid==0){
        printk("kid!\n");
    }else{
        printk("parent\n");
    }
    */
    initVideo(0xFF);
    int flag=1;
    uint32_t old_time=0;
    while(flag){
        unsigned int temp=system_get_tick();
        if(old_time<temp){
            old_time=temp;
            timer();
            int a=system_get_key('a');
            int s=system_get_key('s');
            int w=system_get_key('w');
            int d=system_get_key('d');
            if(a)press(30);
            else if(s)press(31);
            else if(w)press(17);
            else if(d)press(32);
        }
        if(get_gameState()!=1)flag=0;
    }
    printk("LOSE!You live for %d seconds\n",counter3/100);
    while(1);
    return 1;
};
