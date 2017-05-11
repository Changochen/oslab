#include <lib/common.h>
#include <lib/serial.h>
#include <lib/video.h>
#include <lib/keyboard.h>

#include <lib/syscall.h>
#include "inc/process.h"
//extern timer_handler timer_handlers[TIMER_HANDLERS_MAX];
extern uint32_t get_tick();
extern int8_t get_key(char s);
extern void schedule();
extern int fork();

void do_syscall(struct TrapFrame *tf) {
    switch(tf->eax) {
        case SYS_PUTC:
            serial_printc(tf->ebx);
            break;
        case SYS_GET_TICK:
            tf->eax = get_tick();
            break;
        case SYS_GET_KEY:
            tf->eax = get_key(tf->ebx);
            break;
        case SYS_DRAW_POINT:
            tf->eax= __draw_point(tf->ebx,tf->ecx,tf->edx);
            break;
        case SYS_DRAW_LINE:
            tf->eax=__draw_line(tf->ebx,tf->ecx>>16,tf->ecx&0xFFFF,tf->edx);
            break;
        case SYS_DRAW_FRAME:
            __draw_frame();
            break;
        case SYS_GET_POINT:
            tf->eax=__get_point(tf->ebx,tf->ecx);
            break;
        case SYS_GETPID:
            tf->eax=cur_pcb->pid;
            break;
        case SYS_SLEEP:
            cur_pcb->ps=BLOCKED;
            cur_pcb->time_lapse=tf->ebx;
            schedule();
            break;
        case SYS_YIELD:
            cur_pcb->ps=YIELD;
            schedule();
            break;
        case SYS_FORK:
            fork();
            break;
        case SYS_EXIT:
            cur_pcb->inuse=0;
            cur_pcb=NULL;
            schedule();
            break;
        case SYS_FLASH_SCREEN:
            __flash_screen();
            break;
    }
}
