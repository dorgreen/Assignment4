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
int ninodes = 0;

// Each type of command is handled by a different function of type procfs_handler
typedef int (*procfs_handler)(char*);
// this function maps an inode to the appropriate handler
procfs_handler get_handler(struct inode *ip);

// these are the handlers
int handle_ideinfo(char *buff); // Creates the file proc/ideinfo and fills it
int handle_filestat(char *buff); // Creates the file proc/filestat
int handle_inodeinfo(char *buff); // Creates the directory proc/inodeinfo and a file for each inode
int handle_pid_dirent(char *buff); // Creates the PID's dir with files "name" and "status"
int handler_null(char *buff){
    return 0;
}

// Helper functions for getting data
int get_pid(void); // might have a different signature



// init the global var nindoes
void init_ninodes(struct inode *ip){
  if (ninodes != 0) return;

  struct superblock sb;
  readsb(ip->dev, &sb);
  ninodes = sb.ninodes;
  return;
}

// TODO: IMPLEMENT WHEN WE KNOW HOW TO DO THE MAPPINGS!
procfs_handler get_handler(struct inode *ip) {
    return handler_null;
}


// A FILE containing:
//Waiting operations: <Number of waiting operations starting from idequeue>
//Read waiting operations: <Number of read operations>
//Write waiting operations: <Number of write operations>
//Working blocks: <List (#device,#block) that are currently in the queue separated by the ‘;’
// TODO: IMPLEMENT
int handle_ideinfo(char *buff){

    // TODO: FINISH IMPLEMATION IN ide.c
//    int chars_used = get_ideinfo(buff);
    // TODO: FINISH THIS IMPLEMANTATION
    return 0;
}

//a FILE with data from FD table: proc->ofile[]
//  Free fds: <free fd number (ref = 0)>
//  Unique inode fds: <Number of different inodes open by all the fds>
//  Writeable fds: <Writable fd number>
//  Readable fds: <Readable fd number>
//  Refs per fds: <ratio of total number of refs / number of used fds>
// TODO: IMPLEMENT
// TODO: SHOULD WE USE THE STRUCT STAT (stat.h) here? fill it with filestat()
int handle_filestat(char *buff){
//    int pid = get_pid();
//    struct proc* p = get_proc(); // TODO: IMPLEMENT!
//
//
//    // Data to calculate the fields to print
//    int free_fds = 0;
//    int writeable_fds = 0;
//    int readable_fds = 0;
//    int total_refs = 0;
//    int unique_inode = 0;
//    struct inode* used_inodes[16];
//
//    // Fill in this data
//    struct file* proc_file = 0;
//    struct stat st;
//    for(int i = 0 ; i < 16 ; i++){
//       proc_file = p->ofile[i];
//        if(proc_file == 0 || proc_file->type == FD_NONE || proc_file->ref == 0){
//            free_fds++;
//        }
//        else{
//            filestat(proc_file, &st); // fill st with data about proc_file
//            total_refs += st.nlink;
//            //total_refs += proc_file->ref; // TODO: st.nlink or proc_file->ref ?
//
//            if(proc_file->readable) readable_fds++;
//            if(proc_file->writable) writeable_fds++;
//            // Find out if inode is unique
//            int unique = 1;
//            for(int j = 0 ; j < i ; j++){
//                if(used_inodes[j] == proc_file->ip){
//                    unique = 0;
//                    break;
//                }
//            }
//            if(unique){
//                used_inodes[i] = proc_file->ip;
//                unique_inode++;
//            }
//        }
//    }

    // By this line, we have all the data to print :)
    return 0;
}

// resides in DIRECTORY /proc/inodeinfo
// each file within marks a (currently) used inode
// file name: the index in open inode table
// file contents:
//  Device: <device the inode belongs to>
//  Inode number: <inode number in the device>
//  is valid: <0 for no, 1 for yes>
//  type: <DIR, FILE or DEV>
//  major minor: <(major number, minor number)>
//  hard links: <number of hardlinks>
//  blocks used: <number of blocks used in the file, 0 for DEV files>
int handle_inodeinfo(char *buff){
//    int chars_used = 0;
//    for(int i = 0 ; i < NINODE ; i++){
//        // TODO: IMPLEMENT IN fs.c
//        chars_used =+ get_inode_info(buff, i);
//    }
    // TODO: FINISH IMPLEMENTATION HERE!
    return 0;
}



// Check, according to our markings, that ip is indeed a procfs dir.
// Start by checking that ip->major == PROCFS
// also check the minor and inum to make sure it's a dir
int
procfsisdir(struct inode *ip) {
  init_ninodes(ip);
  // Illegal inode for our system!
  if ((ip->type != T_DEV) || (ip->major != PROCFS)){
    return 0;
  }

//  int inum = ip->inum;
//  // TODO: UPDATE HERE WITH MAPPING!
////  if (inum == (ninodes+1) || inum == (ninodes+2))
////    return 0; //blockstat and inodestat are files
////  return (inum < ninodes || inum % 100 == 0 || inum % 100 == 1);
  return 0;
}

//Initialize ip fields.
// upon call, ip only has an inum in it.
// if ip in not valid, the inode will be "read from disk"
// ip->inum is initialized by iget
void
procfsiread(struct inode* dp, struct inode *ip) {
  // TODO: IS THIS REALLY IT??? NOTHING ELSE TO BE DONE?!
    // set minor if needed
    // if ip is a dir, set it's size accordingly
  //ip->valid = 1;
  ip->major = PROCFS;
  ip->type = T_DEV;
}


// THIS WILL RUN WHEN DOING open("proc\..."). [on dirlookup]
// Upon first call, this should create the directory entry of ip, and insert it into dst.
// Upon second call, generate the contents of the given inode, write it into dst
int
procfsread(struct inode *ip, char *dst, int off, int n) {
  init_ninodes(ip);
  char buff[16*66] = {0};
  int chars_used = 0;

  // TODO: IMPLEMENT
  // fill in data so that handler could find the PID
    // int pid = get_pid()
//  short slot = 0;
//  if (ip->inum >= ninodes+100){
//    slot = (ip->inum-ninodes)/100 - 1;
//    ansBuf[0] = slot;
//    short midInum = ip->inum % 100;
//    if (midInum >= 10)
//      ansBuf[1] = midInum-10;
//  }


  procfs_handler handler = get_handler(ip);
  if(handler == handler_null) return 0;
    // Handler will fill buff with relevant data and return the number of chars used

  chars_used = handler(buff);
  memmove((void*)dst, (void*)(*buff+off), n);

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
