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

// CONSTANTS FOR inum => entry mapping
// they are being used s.t inum = ninodes + CONSTANT
// roots
#define ROOT 0
#define IDEINFO 1
#define FILESTAT 2
#define INODEINFO 3
// slots 4 - 63 are used for the entries of /proc/inodeinfo
#define PID 64
// the 0-th entry is used by PID+0 for dir, PID+1 for /name and PID+2 for /status
// the 1-st entry is used by 2*PID+0 for dir, 2*PID+1 for /name and 2*PID+2 for /status


// Counter
int ninodes = 0;

// Each type of command is handled by a different function of type procfs_handler
typedef int (*procfs_handler)(char*);
// this function maps an inode to the appropriate handler
procfs_handler get_handler(struct inode *ip);

// these are the handlers
int handle_root(char* buff); // Creates inodes for the contents of root
int handle_ideinfo(char *buff); // Fill the file proc/ideinfo
int handle_filestat(char *buff); // Fill the file proc/filestat
int handle_inodeinfo(char *buff); // Creates a dir entry for each inode @path proc/inodeinfo
int handle_inodeinfo_entry(char *buff); // Fill the file for the entry with index pointed by the inum
int handle_pid_dirent(char *buff); // Creates the PID's dir with files "name" and "status"
int handle_pid_name(char *buff);
int handle_pid_status(char *buff);
int handler_null(char *buff){
    return 0;
}



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
    int inum = ip->inum;
    if(inum <= ninodes) return handle_root;
    if(inum == ninodes + IDEINFO) return handle_ideinfo;
    if(inum == ninodes + FILESTAT) return handle_filestat;
    if(inum == ninodes + INODEINFO) return handle_inodeinfo;
    if(inum > ninodes + INODEINFO && inum < ninodes + PID) return handle_inodeinfo_entry;
    if(inum > ninodes + PID && ((inum-ninodes)%PID == 0)) return handle_pid_dirent;
    if(inum > ninodes + PID && ((inum-ninodes)%PID == 1)) return handle_pid_name;
    if(inum > ninodes + PID && ((inum-ninodes)%PID == 2)) return handle_pid_status;

    return handler_null;
}


// Upon call, generate inodes for each entry in /proc
// init each entry's INUM such that it could be recognised by get_handler()
int handle_root(char* buff){
  int chars_used = 0;
  int index = 0; // for direntry creation

  // add . and ..
  chars_used += buff_append_dirent(buff, ".", namei("/proc")->inum, index);
  chars_used += buff_append_dirent(buff, "..", namei("")->inum, ++index);

  // Actual files on root
  chars_used += buff_append_dirent(buff, "ideinfo", ninodes + IDEINFO, ++index);
  chars_used += buff_append_dirent(buff, "filestat", ninodes + FILESTAT, ++index);

  // FOLDERS. NOTE LOGIC WITHIN INUM CREATION!
  chars_used += buff_append_dirent(buff, "inodeinfo", ninodes + INODEINFO, ++index);
  // we only need NINODE = 50 spots for entries of /inodeinfo
  // They could occupy spots 4 - 63 ,that is INODEINFO+1 up to PID-1


  // get info and create PID folders
  int used_procs[NPROC];
  get_used_procs(used_procs);
  char pid_buffer[4] = {0}; // Needed to stringify PID

  for(int i = 0 ; i < NPROC ; i++){
      if(used_procs[i] == 0){
        continue;
      }
    // reset, then fill pid_buffer
    memset(pid_buffer, 0 , 4);
    itoa(pid_buffer, used_procs[i]);

    chars_used += buff_append_dirent(buff, pid_buffer, ninodes + ((i+1) * PID), ++index);
    // for proc with index 3 and PID 83:
    // 4*PID is the offset of the dir /83
    // 4*PID + 1 is the offset of the file  /83/name
    // 4*PID + 2 is the offset of the file  /83/status
  }

  return chars_used;
}

// A FILE containing:
//Waiting operations: <Number of waiting operations starting from idequeue>
//Read waiting operations: <Number of read operations>
//Write waiting operations: <Number of write operations>
//Working blocks: <List (#device,#block) that are currently in the queue separated by the ‘;’
int handle_ideinfo(char *buff){
    // TODO: TEST IMPLEMATION IN ide.c
    return get_ideinfo(buff);
}

// a FILE with data from FD table @ file.c
//  Free fds: <free fd number (ref = 0)>
//  Unique inode fds: <Number of different inodes open by all the fds>
//  Writeable fds: <Writable fd number>
//  Readable fds: <Readable fd number>
//  Refs per fds: <ratio of total number of refs / number of used fds>
int handle_filestat(char *buff){
    // TODO: TEST IMPLEMANTATION @ file.c
    return get_filestat(buff);
}

// DIRECTORY /proc/inodeinfo
// each file within marks a (currently) used inode
// file name: the index in open inode table
int handle_inodeinfo(char *buff){
    int used_inode_indcies[NINODE] = {0};
    get_used_inode_count(used_inode_indcies);

    int chars_used = 0;
    int index = 0; // for direntry creation

    // add . and ..
    chars_used += buff_append_dirent(buff, ".", namei("/proc/")->inum, index);
    chars_used += buff_append_dirent(buff, "..", namei("")->inum, ++index);

    // Actual files on root
    char index_buffer[4] = {0}; // Needed to stringify PID
    for(int i = 0 ; i < NINODE ; i++){
        if(used_inode_indcies[i] == 0) continue;

        // reset, then fill index_buffer
        memset(index_buffer, 0 , 4);
        itoa(index_buffer, used_inode_indcies[i]);
        // CREATE DIR ENTRY
        chars_used += buff_append_dirent(buff, index_buffer, ninodes + INODEINFO+1+i, ++index);
    }

    return chars_used;
}

// file name: the index in open inode table
// file contents:
//  Device: <device the inode belongs to>
//  Inode number: <inode number in the device>
//  is valid: <0 for no, 1 for yes>
//  type: <DIR, FILE or DEV>
//  major minor: <(major number, minor number)>
//  hard links: <number of hardlinks>
//  blocks used: <number of blocks used in the file, 0 for DEV files>
int handle_inodeinfo_entry(char* buff){
    int entry = (int) buff[0];
    buff[0] = 0;
    return get_inode_info(buff, entry);
}

// Creates the PID's dir with files "name" and "status"
int handle_pid_dirent(char *buff){
    int ptable_index = buff[0];
    buff[0] = 0;

    int chars_used = 0;
    int index = 0; // for direntry creation

    // add . and ..
    chars_used += buff_append_dirent(buff, ".", namei("/proc")->inum, index);
    chars_used += buff_append_dirent(buff, "..", namei("")->inum, ++index);

    // Actual files on root
    chars_used += buff_append_dirent(buff, "name", ninodes + 1 + ((1+ptable_index) * PID) , ++index);
    chars_used += buff_append_dirent(buff, "status", ninodes + 2 + ((1+ptable_index) * PID), ++index);

    return chars_used;
}

// fill in the file /name
int handle_pid_name(char *buff){
    int ptable_index = buff[0];
    buff[0] = 0;

    return get_proc_name(ptable_index, buff);
}

// fill in the file /status
int handle_pid_status(char *buff){
    int ptable_index = buff[0];
    buff[0] = 0;

    return get_proc_status(ptable_index, buff);
}

// Check, according to our markings, that ip is indeed a procfs dir.
// Start by checking that ip->major == PROCFS
// The only folders are root, inodeinfo, and one for each proc.
int
procfsisdir(struct inode *ip) {
    init_ninodes(ip);

    // check if inode is illeagal for our system!
    if ((ip->type != T_DEV) || (ip->major != PROCFS)){
       return 0;
    }

    int inum = ip->inum;

    // case : root
    if(inum < ninodes || namei("/proc")->inum == inum) return 1;
    // case: inodeinfo
    if(inum == ninodes + INODEINFO) return 1;
    // case: a dir for some proc i
    if(inum - ninodes >= PID && (inum-ninodes % PID == 0)) return 1;

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
    ip->valid = 1;
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

    procfs_handler handler = get_handler(ip);

    if(handler == handler_null) return 0;

      // Fill buff with relevant data for handler, if needed
    if(handler == handle_inodeinfo_entry){
        // Set first char of buff to be the inode_table index of requested inode
        buff[0] = ip->inum - ninodes - INODEINFO - 1;
    }

    else if(handler == handle_pid_dirent || handler == handle_pid_name || handler == handle_pid_status){
        // Set first char of buff to be the ptable index of requested proc
        buff[0] = (char) (ip->inum - ninodes) / PID;
    }


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
