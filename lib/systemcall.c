#include<lib/common.h>
#include<lib/syscall.h>
#include<inc/types.h>
#include<inc/string.h>
#define VGA_SIZ 64000
uint8_t* VGAP=(uint8_t*)0xa0000;
uint8_t Frame[320*200];



unsigned int system_get_tick(){
    uint32_t t;
    asm volatile("int $0x80": "=a"(t) : "a"(SYS_GET_TICK));
    return t;
}

int fs_open(const char *pathname, int flags){
    int success;
    asm volatile("int $0x80": "=a"(success) : "a"(SYS_OPEN),"b"(pathname),"c"(flags));
    return success;
}

int fs_read(int fd, void *buf, int len){
    int read_byte;
    asm volatile("int $0x80": "=a"(read_byte) : "a"(SYS_READ),"b"(fd),"c"(buf),"d"(len));
    return read_byte;
}

int fs_write(int fd, void *buf, int len){
    int write_byte;
    asm volatile("int $0x80": "=a"(write_byte) : "a"(SYS_WRITE),"b"(fd),"c"(buf),"d"(len));
    return write_byte;
}

int fs_lseek(int fd, int offset, int whence){
    int success;
    asm volatile("int $0x80": "=a"(success) : "a"(SYS_LSEEK),"b"(fd),"c"(offset),"d"(whence));
    return success;
}

int fs_close(int fd){
    int success;
    asm volatile("int $0x80": "=a"(success) : "a"(SYS_CLOSE),"b"(fd));
    return success;
}
void system_thread_create(uint32_t func){
    asm volatile("int $0x80": : "a"(SYS_PTHREAD_CREATE),"b"(func));
}

void system_exec(char* command){
    asm volatile("int $0x80": : "a"(SYS_EXEC),"b"(command));
}

void system_sem_destroy(void *sem){
    asm volatile("int $0x80": : "a"(SYS_SEM_DESTROY),"b"(sem));
}

void system_sem_post(void *sem){
    asm volatile("int $0x80": : "a"(SYS_SEM_POST),"b"(sem));
}

void system_sem_wait(void *sem){
    asm volatile("int $0x80": : "a"(SYS_SEM_WAIT),"b"(sem));
}

int system_sem_trywait(void* sem){
    int res;
    asm volatile("int $0x80":"=a"(res) : "a"(SYS_SEM_TRYWAIT),"b"(sem));
    return res;
}

void system_sem_init(void* sem,int count){
    asm volatile("int $0x80": : "a"(SYS_SEM_INIT),"b"(sem),"c"(count));
}

char system_key_down(char s){
    char r_eax = 0;
    asm volatile("int $0x80": : "a"(SYS_GET_KEY), "b"(s));
    asm volatile("movl %%eax, %0\n" : : "m"(r_eax));
    return r_eax;
}

int __draw_point(int x,int y,int color){
    if(x<0||x>=200||y<0||y>=320)return -1;
    Frame[320*x+y]=color&0xff;
    return 1;
}

int __draw_line(int bx,int by,int ey,int color){
    if(bx<0||bx>=200||by<0||by>=320||ey<0||ey>=320||by>ey)return -1;
    while(by<=ey){
        Frame[320*bx+by]=color&0xff;
        by++;
    }
    return 1;
}

void __draw_frame(){
    memcpy(VGAP,Frame,VGA_SIZ);
}

void __flash_screen(){
    memset(VGAP,-1,VGA_SIZ);
}
int system_fork(){
    int r_eax=0;
    asm volatile("int $0x80":"=a"(r_eax):"a"(SYS_FORK));
    return r_eax;
}

void system_exit(){
    asm volatile("int $0x80"::"a"(SYS_EXIT));
}
char system_get_key(char c){
    char r_eax = 0;
    asm volatile("int $0x80":"=a"(r_eax) : "a"(SYS_GET_KEY), "b"(c));
    return r_eax;
}
void system_putc(char c){
    asm volatile("int $0x80": : "a"(SYS_PUTC), "b"(c));
}

int system_draw_point(int x,int y,int color){
    int res=0;
    asm volatile("int $0x80": "=a"(res) : "a"(SYS_DRAW_POINT),"b"(x),"c"(y),"d"(color));
    return res;
}

int system_draw_line(int bx,int by,int ey,int color){
    int res=0;
    asm volatile("int $0x80": "=a"(res) : "a"(SYS_DRAW_LINE),"b"(bx),"c"((by<<16)+ey),"d"(color));
    return res;
}

void system_draw_frame(){
    asm volatile("int $0x80":: "a"(SYS_DRAW_FRAME));
}

char __get_point(int x,int y){
    return Frame[320*x+y];
}
char system_get_point(int x,int y){
    int res=0;
    asm volatile("int $0x80":"=a"(res):"a"(SYS_GET_POINT),"b"(x),"c"(y));
    return res&0xFF;
}

void system_yield(){
    asm volatile("int $0x80"::"a"(SYS_YIELD));
}

void system_sleep(uint32_t sec){
    asm volatile("int $0x80"::"a"(SYS_SLEEP),"b"(sec));
}

uint32_t system_getpid(){
    uint32_t res;
    asm volatile("int $0x80":"=a"(res):"a"(SYS_GETPID));
    return res;
}

void system_flash_screen(){
    asm volatile("int $0x80"::"a"(SYS_FLASH_SCREEN));
}
