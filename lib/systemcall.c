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
    asm volatile("int $0x80":"=a"(res):"a"(SYS_GET_POINT),"b"(x),"c"(y));//????
    return res&0xFF;
}

