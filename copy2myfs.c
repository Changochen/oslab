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

int get_file_offset(char* filename){
    for(int i=0;i!=32;i++){
        printf("%s",dir[i].filename);
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
    if(argc!=2){
        printf("Usage :copy2myfs newfile\n");
        return -1;
    }
    int disks=open("disks.bin",O_RDWR);
    lseek(disks,SEC,SEEK_SET);
    read(disks,bitmap,sizeof(bitmap));
    printf("%c\n",bitmap[0]);
    read(disks,dir,sizeof(dir));
    read(disks,inodes,sizeof(inodes));

    int newfile=open(argv[1],O_RDONLY);
    if(get_file_offset(argv[1])!=-1){
        printf("File exists!\n");
        close(disks);
        return -1;
    }

    int file_offset=get_new_file_offset();
    if(file_offset==-1){
        printf("Max files !\n");
        close(disks);
        return -1;
    }
    printf("%d\n",file_offset);
    memcpy(dir[file_offset].filename,argv[1],strlen(argv[1]));
    printf("%s\n",dir[file_offset].filename);
    dir[file_offset].inode_offset=file_offset;
    dir[file_offset].file_size=lseek(newfile,0,SEEK_END);
    printf("%d\n",dir[file_offset].file_size);
    lseek(newfile,0,SEEK_SET);
    lseek(disks,0,SEEK_END);
    int count=0;
    while(1){
        memset(buffer,0,SEC);
        if(read(newfile,buffer,SEC)<=0)break;
        unsigned int tmpoff=get_offset(disks);
        set_bitmap(tmpoff);
        inodes[file_offset].blocks[count++]=tmpoff;
        write(disks,buffer,SEC);
    }
    close(newfile);
    lseek(disks,SEC,SEEK_SET);
    write(disks,bitmap,sizeof(bitmap));
    write(disks,dir,sizeof(dir));
    write(disks,inodes,sizeof(inodes));
    close(disks);
    return 0;
}

