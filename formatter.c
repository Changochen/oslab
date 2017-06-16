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

inode inodes[32];
typedef struct {
    char     filename[24];
    unsigned int file_size;
    unsigned int inode_offset;

}dirent;

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

int main(){
    int boot=open("boot.bin",O_RDONLY);
    system("rm disks.bin");
    system("touch disks.bin");
    int disks=open("disks.bin",O_WRONLY);
    read(boot,buffer,SEC);
    close(boot);
    write(disks,buffer,SEC);

    /*
    lseek(disks,(DATA_OFFSET+1)*SEC,SEEK_SET);
    int kernel=open("ker.bak",O_RDONLY);
    while(1){
        memset(buffer,0,SEC);
        if(read(kernel,buffer,SEC)<=0)break;
        set_bitmap(get_offset(disks));
        write(disks,buffer,SEC);
    }
    close(kernel);
    */
    lseek(disks,SEC,SEEK_SET);
    write(disks,bitmap,sizeof(bitmap));
    write(disks,dir,sizeof(dir));
    write(disks,inodes,sizeof(inodes));
    close(disks);
    system("./copy2myfs ker.bak");
    system("./copy2myfs game.bin");
    return 0;
}

