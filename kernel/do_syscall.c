#include <lib/common.h>
#include <lib/serial.h>
#include <lib/video.h>
#include <lib/keyboard.h>

#include <lib/syscall.h>

extern timer_handler timer_handlers[TIMER_HANDLERS_MAX];
extern uint32_t get_tick();
extern int8_t get_key(char s);

void do_syscall(struct TrapFrame *tf) {
	//int i;
	switch(tf->eax) {
		case SYS_GET_TICK:
			tf->eax = get_tick();
		break;
		case SYS_GET_KEY:
			tf->eax = get_key(tf->ebx);
		break;
	}
}
