#include <lib/common.h>
#include <lib/serial.h>
#include <lib/video.h>
#include <lib/keyboard.h>

#include <lib/syscall.h>

extern timer_handler timer_handlers[TIMER_HANDLERS_MAX];
extern uint32_t get_tick();
extern int8_t get_key(char s);

void do_syscall(struct TrapFrame *tf) {
    switch(tf->eax) {
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
    }
}
