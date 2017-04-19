#include<lib/video.h>
#include<inc/string.h>
#include<lib/random.h>
#include<lib/blocklist.h>
#include<lib/syscall.h>
#include<lib/common.h>
int gameState=0;
//uint8_t Frame[320*200];
int x,y;
int num=0;
//uint8_t* VGAP=(uint8_t*)0xa0000;
void drawFrame(){
    system_draw_frame();
}

uint8_t K[10];

void drawBlock(int x,int y,uint8_t color){
    for(int i=0;i<10;i++){
        int k=system_draw_line(x+i,y,y+9,color);
        if(k==-1){
            printf("draw failed!x:%d,y:%d\n",x+i,y);
        }
    }
}

int get_gameState(){
    return gameState;
}

void initVideo(uint8_t color){
    for(int i=0;i<320;i++){
        for(int j=0;j<200;j++){
            system_draw_point(j,i,0xff);
        }
    }
    gameState=1;
    x=190;
    y=150;
    for(int i=0;i<10;i++){
        K[i]=color;
    }
    init_list();
    drawBlock(x,y,MBCOLOR);
}

void addBlock(){
    add(0,(rand()%32)*10);
}

void randomMove(){
    NODE* head=get_head();
    while (head!=NULL){
        if(head->x==190){
            drawBlock(head->x, head->y, 0xFF);
            head=del(head);
            continue;
        }
        else
        {
            moveBlock(&head->x, &head->y, DOWN, 0x23,1);
            moveBlock(&head->x, &head->y, rand() % 2 + 1, BCOLOR,1);
        }
    head=head->next;
    }
}

void moveBlock(int *nx, int *ny, int direction, uint8_t color,int which)
{
    drawBlock(*nx, *ny, 0xFF);
    switch (direction)
    {
        case UP:
            if (*nx != 0)
            {
                *nx -= 10;
            }
            break;
        case DOWN:
            if (*nx != 190)
            {
                *nx += 10;
            }
            break;
        case RIGHT:
            if (*ny != 310)
            {
                *ny += 10;
            }
            break;
        case LEFT:
            if (*ny != 0)
            {
                *ny -= 10;
            }
            break;
    }
    if((which==1&&(system_get_point(*nx,*ny)==MBCOLOR))||(which==0&&(system_get_point(*nx,*ny)==BCOLOR))){
        gameState=0;
    }
    drawBlock(*nx, *ny, color);
}

void move(int direction)
{
    moveBlock(&x, &y, direction, MBCOLOR,0);
}
