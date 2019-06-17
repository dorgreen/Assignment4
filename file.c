//
// File descriptors
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE)
    pipeclose(ff.pipe, ff.writable);
  else if(ff.type == FD_INODE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  if(f->type == FD_INODE){
    ilock(f->ip);
    stati(f->ip, st);
    iunlock(f->ip);
    return 0;
  }
  return -1;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}

// a FILE with data from FD table @ file.c
// @PROCFS
// TODO: TEST
// TODO: SHOULD WE USE THE STRUCT STAT (stat.h , field nlink) here? [fill it with filestat()]
int get_filestat(char* buff){
  int free_fds = 0;
  int writeable_fds = 0;
  int readable_fds = 0;
  int total_refs = 0;
  int unique_inode = 0;
  struct inode* used_inodes[NFILE];

  // Fill in this data
  struct file* this_file = 0;

  acquire(&ftable.lock);

  for(int i = 0 ; i < NFILE ; i++){
    this_file = &ftable.file[i];
    if(this_file == 0 || this_file->type == FD_NONE || this_file->ref == 0){
      free_fds++;
    }
    else{
      total_refs += this_file->ref; // TODO: st.nlink or proc_file->ref ?

      if(this_file->readable) readable_fds++;
      if(this_file->writable) writeable_fds++;
      // Find out if inode is unique
      int unique = 1;
      for(int j = 0 ; j < unique_inode ; j++){
        if(used_inodes[j] == this_file->ip){
          unique = 0;
          break;
        }
      }
      if(unique){
        used_inodes[unique_inode] = this_file->ip;
        unique_inode++;
      }
    }
  }

  release(&ftable.lock);
  //  Free fds: <free fd number (ref = 0)>
//  Unique inode fds: <Number of different inodes open by all the fds>
//  Writeable fds: <Writable fd number>
//  Readable fds: <Readable fd number>
//  Refs per fds: <ratio of total number of refs / number of used fds>
  int chars_used = 0;
  chars_used += buff_append(buff,"Unique idnode fds: ");
  chars_used += buff_append_num(buff, unique_inode);
  chars_used += buff_append(buff,"\nWriteable fds: ");
  chars_used += buff_append_num(buff, writeable_fds);
  chars_used += buff_append(buff,"\nReadable fds: ");
  chars_used += buff_append_num(buff, readable_fds);
  chars_used += buff_append(buff,"\nRefs per fds: ");
  chars_used += buff_append_num(buff, total_refs);
  chars_used += buff_append(buff," / ");
  chars_used += buff_append_num(buff, NFILE - free_fds);
  chars_used += buff_append(buff,"\n");

  return chars_used;
}
