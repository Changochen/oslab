#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
/*
+------------+--------+-------+-------+-------+-------+-------+
| bootloader | bitmap |  dir  | inode | data  |  ...  |  ...  |
+------------+--------+-------+-------+-------+-------+-------+
1 sec           64 sec  2 sec   32 sec
*/

#define SEC 512
#define DATA_OFFSET (64+2+32)
#pragma pack(0)
typedef struct
{
    unsigned int blocks[128];

}inode;

typedef struct {
    char     filename[24];
    unsigned int file_size;
    unsigned int inode_offset;

}dirent;

inode inodes[32];
dirent dir[512 / sizeof(dirent)*2];

unsigned char bitmap[512 * 64];  // 512B ~ 2MB

#pragma pack()
char buffer[SEC];

unsigned char bits[]={
    0x80,0x40,0x20,0x10,8,4,2,1
};

void set_bitmap(unsigned int offset){
    unsigned int low=offset&7;
    offset>>=3;
    bitmap[offset]|=bits[low];
}

unsigned int get_offset(int fd){
    off_t currpos;
    currpos = lseek(fd, 0, SEEK_CUR);
    currpos/=512;
    currpos=currpos-(DATA_OFFSET)-1;
    return currpos;
}

void go_to_block(int fd,unsigned offset){
    lseek(fd,(1+DATA_OFFSET+offset)*SEC,SEEK_SET);
}

int get_file_offset(char* filename){
    for(int i=0;i!=32;i++){
        if(strcmp(filename,dir[i].filename)==0)return i;
    }
    return -1;
}

int get_new_file_offset(){
    for(int i=0;i!=32;i++){
        if(dir[i].file_size==0)return i;
    }
    return -1;
}
int main(int argc,char** argv){
    if(argc!=3){
        printf("Usage :read_myfs readfile save_file\n");
        return -1;
    }
    int disks=open("disks.bin",O_RDONLY);
    lseek(disks,SEC,SEEK_SET);
    read(disks,bitmap,sizeof(bitmap));
    read(disks,dir,sizeof(dir));
    read(disks,inodes,sizeof(inodes));

    int readfile_off;
    if((readfile_off=get_file_offset(argv[1]))==-1){
        printf("File not exists!\n");
        close(disks);
        return -1;
    }

    int sec=(dir[readfile_off].file_size>>9)+1;
    char* file_buffer=(char*)malloc(dir[readfile_off].file_size);
    char* temp=file_buffer;
    int readfile=open(argv[2],O_WRONLY|O_CREAT,0600);
    //int readfile=open(argv[2],O_WRONLY);
    int byte_left=dir[readfile_off].file_size;
    for(int i=0;i!=sec;i++){
        memset(buffer,0,SEC);
        //printf("%x\n",inodes[readfile_off].blocks[i]);
        go_to_block(disks,inodes[readfile_off].blocks[i]);
        read(disks,buffer,SEC);
        memcpy(file_buffer,buffer,(byte_left>512?512:byte_left));
        file_buffer+=512;
        byte_left-=512;
    }
    write(readfile,temp,dir[readfile_off].file_size);
    close(readfile);
    close(disks);
    return 0;
}

