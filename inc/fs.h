#ifndef __FS_H__
#define __FS_H__
//#include <inc/fs.h>
//#include <inc/lib.h>
#include <inc/types.h>
#define SECTSIZE	512			// bytes per disk sector
#define BLKSECTS	(BLKSIZE / SECTSIZE)	// sectors per block

/* Disk block n, when in memory, is mapped into the file system
 * server's address space at DISKMAP + (n*BLKSIZE). */
#define DISKMAP		0x10000000

/* Maximum disk size we can handle (3GB) */
#define DISKSIZE	0xC0000000

#pragma pack(0)
typedef struct
{
	unsigned int blocks[128];
}inode;

typedef struct {
  char     filename[24];
  uint32_t file_size;
  uint32_t inode_offset;
}dirent;

typedef struct {
  dirent entries[512 / sizeof(dirent)];
}dir;

typedef struct {
  uint8_t mask[512 * 64];  // 512B ~ 2MB
}bitmap;
#pragma pack()

typedef int bool;
//struct Super *super;		// superblock
//uint32_t *bitmap;		// bitmap blocks mapped in memory

/* ide.c */
bool	ide_probe_disk1(void);
void	ide_set_disk(int diskno);
void	ide_set_partition(uint32_t first_sect, uint32_t nsect);
int	ide_read(uint32_t secno, void *dst, size_t nsecs);
int	ide_write(uint32_t secno, const void *src, size_t nsecs);


int open(const char *pathname, int flags);
int read(int fd, void *buf, int len);
int write(int fd, void *buf, int len);
int lseek(int fd, int offset, int whence);
int close(int fd);
#endif
