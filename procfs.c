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


// Check, according to our markings, that ip is indeed a procfs dir.
// Start by checking that ip->major == PROCFS
// also check the minor and inum to make sure it's a dir
int 
procfsisdir(struct inode *ip) {
  return 0;
}

//Initialize ip fields
void
procfsiread(struct inode* dp, struct inode *ip) {
}


// THIS WILL RUN WHEN DOING open("proc\..."). [on dirlookup]
// Upon first call, this should create the directory entry of ip, and insert it into dst.
// Upon second call, generate the contents of the given inode, write it into dst
int
procfsread(struct inode *ip, char *dst, int off, int n) {
  return 0;
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
