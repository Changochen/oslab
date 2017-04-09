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
		/*
		case SYS_PRINT_CHAR:
			serial_printc(tf->ebx);
		break;
		case SYS_INIT_CACHE:
			initVCache();
		break;
		case SYS_CLEAR_VRAM:
			clearVRAM();
		break;
		case SYS_FLUSH_VCACHE:
			clearVRAM();
			flushVCache();
		break;
		case SYS_SET_PIXEL:
			setPixelAt(tf->ebx, tf->ecx, (uint8_t)tf->edx);
		break;
		case SYS_GET_KEY:
			tf->eax = get_key(tf->ebx);
		break;
		*/
		case 4000:
			tf->eax = get_tick();
		break;
		case 4001:
			tf->eax = get_key(tf->ebx);
		break;
		/*
		case SYS_ADD_TIMER:
			for(i=0;i<TIMER_HANDLERS_MAX;i++){
				if(!timer_handlers[i].used){
					timer_handlers[i].ptr = (void*)tf->ebx;
					timer_handlers[i].used = 1;
					break;
				}
			}
		break;
		*/
	}
}
