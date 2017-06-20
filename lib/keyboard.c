#include "keyboard.h"
#include "lib/serial.h"
#include "lib/common.h"
#define LETTER_NUM 30
static int letter[] = {
	30, 48, 46, 32, 18, 33, 34, 35, 23, 36,
	37, 38, 50, 49, 24, 25, 16, 19, 31, 20,
	22, 47, 17, 45, 21, 44, 57, 14, 28, 52
};
static int printable_letter[]={
	'a','b','c','d','e','f','g','h','i','j',
	'k','l','m','n','o','p','q','r','s','t',
	'u','v','w','x','y','z',' ','\b','\n','.'
};
static int keydown[LETTER_NUM]={0};

static int lastkey;

int8_t get_key(char s){
	return keydown[s-'a'];
}

static int roffset=0;
static int woffset=0;
static char buffer[1024];

int8_t getc(){
	if(roffset==1024)roffset=0;
	while(roffset==woffset);
	if(buffer[roffset]=='\b'){
		serial_printc('\b');
		serial_printc(' ');
	}
	return buffer[roffset++];
}
int8_t get_lastkey(){
	return lastkey+'a';
}
void press_key(int code){
	int i=0;
	if((code & 0x80) == 0){
		for(;i<LETTER_NUM;i++){
			if(letter[i]==code){
				keydown[i] = 1;
				lastkey = i;
				buffer[woffset++]=printable_letter[i];
				if(woffset==1024)woffset=0;
				return;
			}
		}
	}else{
		code &= 0x7F;
		for(;i<LETTER_NUM;i++){
			if(letter[i]==code){
				keydown[i] = 0;
				return;
			}
		}
	}
}
