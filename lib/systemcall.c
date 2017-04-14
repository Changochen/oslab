#include<lib/common.h>
#include<lib/syscall.h>
unsigned int system_get_tick(){
    uint32_t t;
    asm volatile("int $0X80": "=a"(t) : "a"(SYS_GET_TICK));
    return t;
}

char system_key_down(char s){
	char r_eax = 0;
	asm volatile("int $0x80": : "a"(SYS_GET_KEY), "b"(s));
	asm volatile("movl %%eax, %0\n" : : "m"(r_eax));
	return r_eax;
}