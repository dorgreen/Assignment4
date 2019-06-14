#include "types.h"
#include "stat.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// Counter
int nindoes = 0;

// Each type of command is handled by a different function
// all of them are of the same type to simplify code
typedef int *(procfs_handler)(char*);

// these are the handlers
int handle_ideinfo(char *buff);
int handle_filestat(char *buff);
int handle_inodeinfo(char *buff);
int handle_pid_dirent(char *buff); // creates the PID's dir with both name and status

// this function maps an inode to the appropriate handler
procfs_handler get_handler(struct inode *ip);

// Helper functions for string handling
void buff_append(char *buff, char *data);
void buff_append_num(char *buff, int data);
void buff_append_dirent(char *buff, char * dir, int inum, int dir_offset);
void itoa(char* string, int num); // Num to string. maybe use .format instead


// init the global var nindoes
void init_ninodes(void){
  if (ninodes != 0) return;

  struct superblock sb;
  readsb(ip->dev, &sb);
  ninodes = sb.ninodes;
  return;

}


// Check, according to our markings, that ip is indeed a procfs dir.
// Start by checking that ip->major == PROCFS
// also check the minor and inum to make sure it's a dir
int
procfsisdir(struct inode *ip) {
  init_ninodes();
  // Illegal inode for our system!
  if ((ip->type != T_DEV) || (ip->major != PROCFS)){
    return 0;
  }

  int inum = ip->inum;
  // TODO: UPDATE HERE WITH MAPPING!
//  if (inum == (ninodes+1) || inum == (ninodes+2))
//    return 0; //blockstat and inodestat are files
//  return (inum < ninodes || inum % 100 == 0 || inum % 100 == 1);
  return 0;
}

//Initialize ip fields
void
procfsiread(struct inode* dp, struct inode *ip) {
  // TODO: IS THIS REALLY IT??? NOTHING ELSE TO BE DONE?!
  ip->flags |= I_VALID;
  ip->major = PROCFS;
  ip->type = T_DEV;
}


// THIS WILL RUN WHEN DOING open("proc\..."). [on dirlookup]
// Upon first call, this should create the directory entry of ip, and insert it into dst.
// Upon second call, generate the contents of the given inode, write it into dst
int
procfsread(struct inode *ip, char *dst, int off, int n) {
  init_ninodes();

  char buff[16*66] = {0};
  int chars_used = 0;

  // TODO: IMPLEMENT
  // fill in data so that handler could find the PID
//  short slot = 0;
//  if (ip->inum >= ninodes+100){
//    slot = (ip->inum-ninodes)/100 - 1;
//    ansBuf[0] = slot;
//    short midInum = ip->inum % 100;
//    if (midInum >= 10)
//      ansBuf[1] = midInum-10;
//  }


  procfs_handler handler = get_handler(ip);
  if(handler == 0) return 0;

  chars_used = handler(buff);
  memmove(dst, buff+off, n);

  if(chars_used-off < n){
    return chars_used-off;
  }
  else{
    return n;
  }
}


// ALWAYS 0 as our system is read-only.
int
procfswrite(struct inode *ip, char *buf, int n)
{
  return 0;
}

void
procfsinit(void)
{
  devsw[PROCFS].isdir = procfsisdir;
  devsw[PROCFS].iread = procfsiread;
  devsw[PROCFS].write = procfswrite;
  devsw[PROCFS].read = procfsread;
}
