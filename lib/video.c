#include<lib/video.h>
#include<inc/string.h>
#include<lib/random.h>
uint8_t Frame[320*200];
int x,y;
int num=0;
uint8_t* VGAP=(uint8_t*)0xa0000;
void drawFrame(){
    memcpy(VGAP,Frame,VGA_SIZ);
}

int randomBlock[1000][2];
uint8_t K[10];

void drawBlock(int x,int y,uint8_t color){
    for(int i=0;i<10;i++){
        K[i]=color;
    }
    for(int i=0;i<10;i++){
        memcpy((Frame+(x+i)*320+y),K,10);
    }
}

void initVideo(uint8_t color){
    x=190;
    y=150;
    for(int i=0;i<10;i++){
        K[i]=color;
    }
    drawBlock(0,0,MBCOLOR);
}

void addBlock(){
    randomBlock[num][0]=0;
    randomBlock[num][1]=(rand()%32)*10;
    num++;
}

void randomMove(){
    int index=0;
    for(int i=num;i>0;i--){
        while(1){
            if(randomBlock[index][1]!=-1){
                moveBlock(&randomBlock[index][0],&randomBlock[index][1],DOWN,0x23);
                if(randomBlock[index][1]==190){
                    randomBlock[index][1]=-1;
                    drawBlock(randomBlock[index][0],randomBlock[index][1],0xFF);
                    num--;
                }else{
                    moveBlock(&randomBlock[index][0],&randomBlock[index][1],rand()%2+1,BCOLOR);
                }
                index++;
                break;
            }
        }
    }
}
void moveBlock(int* nx,int* ny,int direction,uint8_t color){
    drawBlock(*nx,*ny,0xFF);
    switch(direction){
        case UP:
            if(*nx!=0){
                *nx-=10;
            }
            break;
        case DOWN:
            if(*nx!=190){
                *nx+=10;
            }
            break;
        case RIGHT:
            if(*ny!=310){
                *ny+=10;
            }
            break;
        case LEFT:
            if(*ny!=0){
                *ny-=10;
            }
            break;
    }
    drawBlock(*nx,*ny,color);
}

void move(int direction){
    moveBlock(&x,&y,direction,MBCOLOR);
}
