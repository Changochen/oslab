#include<inc/types.h>
#define VGA_SIZ 64000
#define UP      4
#define DOWN    3
#define RIGHT   2
#define LEFT    1
#define MBCOLOR 0x5F
#define BCOLOR  0x23
void initVideo(uint8_t);
void drawFrame();
void move(int);
void randomMove();
void addBlock();
void moveBlock(int*,int*,int,uint8_t,int);
int get_gameState();
