#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib/common.h"

#define SYS_GET_TICK	4000
#define SYS_GET_KEY		4001
#define SYS_DRAW_POINT  4002
#define SYS_DRAW_LINE	4003
#define SYS_DRAW_FRAME	4004
#define SYS_GET_POINT   4005
#define SYS_YIELD       4006
#define SYS_SLEEP       4007
#define SYS_GETPID      4008
#define SYS_PUTC    	4009
#define SYS_FORK        4010
#define SYS_EXIT        4011
#define SYS_FLASH_SCREEN 4012
#define SYS_SEM_INIT    4013
#define SYS_SEM_DESTROY   4014
#define SYS_SEM_WAIT    4015
#define SYS_SEM_POST    4016
#define SYS_PTHREAD_CREATE 4017
#define SYS_SEM_TRYWAIT  4018
#define TIMER_HANDLERS_MAX 100
#define SYS_OPEN		4019
#define SYS_CLOSE		4020
#define SYS_READ 		4021
#define SYS_WRITE		4022
#define SYS_LSEEK		4023


unsigned int system_get_tick();
char system_get_key(char);
int system_draw_point(int x,int y,int color);
int system_draw_line(int bx,int by,int ey,int color);
void system_flash_screen();
void system_draw_frame();
int __draw_point(int x,int y,int color);
int __draw_line(int bx,int by,int ey,int color);
void __draw_frame();
void __flash_screen();
char system_get_point(int,int);
char __get_point(int,int);
void system_yield();
void system_sleep(uint32_t);
uint32_t system_getpid();
void system_putc(char c);
int system_fork();
void system_exit();
void system_sem_init(void *,int count);
void system_sem_destroy(void*);
void system_sem_wait(void *);
void system_sem_post(void *);
int system_sem_trywait(void*);
void system_thread_create(uint32_t func);
int fs_open(const char *pathname, int flags);
int fs_read(int fd, void *buf, int len);
int fs_write(int fd, void *buf, int len);
int fs_lseek(int fd, int offset, int whence);
int fs_close(int fd);
typedef struct timer_handler{
    void (*ptr)(void);
    int used;
}timer_handler;

#endif
