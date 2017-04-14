#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib/common.h"

#define SYS_PRINT_CHAR	1000

#define SYS_GET_TICK	4000
#define SYS_GET_KEY		4001
#define SYS_DRAW_POINT  4002
#define SYS_DRAW_LINE	4003
#define SYS_DRAW_FRAME	4004

#define TIMER_HANDLERS_MAX 100


unsigned int system_get_tick();
char system_get_key(char);
void system_draw_point(int x,int y,int color);
void system_draw_line(int bx,int by,int ex,int ey,int color);
void system_draw_frame();
typedef struct timer_handler{
	void (*ptr)(void);
	int used;

}timer_handler;
#endif