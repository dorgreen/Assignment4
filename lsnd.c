//
// Created by Dor Green
//
//Write a user program called “lsnd” that will pull the following information for each inode
//in use:
//<#device> <#inode> <is valid> <type> <(major,minor)> <hard links> <blocks used>
#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#define SZ 64


int itoa(char* string, int num){
    int i = 0;
    int len = 0;
    if (num == 0){
        string[0] = '0';
        return 1;
    }

    while(num != 0){
        string[len] = num % 10 + '0';
        num = num / 10;
        len++;
    }
    for(i = 0; i < len/2; i++){
        char tmp = string[i];
        string[i] = string[len - 1 - i];
        string[len - 1 - i] = tmp;
    }
    return len;
}

int buff_append(char *buff, char *data){
    int current_length = strlen(buff);
    int length_to_add = strlen(data);

    memmove(buff + current_length, data, length_to_add);
    return length_to_add;
}

int buff_append_num(char *buff, int data){
    char int_buff[4] = {0};
    itoa(int_buff, data);
    int chars_written = buff_append(buff, int_buff);
    return chars_written;
}

void reset_path_buff(char* buff){
    memset(buff,0, SZ);
    buff_append(buff, "/proc/inodeinfo/");
    return;
}


// should read from the folder /proc/inodeinfo
// each file in that folder has the data we need for each used inode :)
int
main(int argc, char *argv[]) {
    int fd = 0;
    char path_buff[SZ] = {0};
    char ans_buff[1024] = {0};

    for(int i = 0 ; i < NINODE ; i++){
        // reset
        reset_path_buff(path_buff);
        buff_append_num(path_buff,i);

        // open
        fd = open(path_buff,O_RDWR);
        if(fd < 0) continue;

        // read & print
        memset(ans_buff,0, 1024);
        if(read(fd, ans_buff, 1024) > 0){
            printf(1, ans_buff);
            printf(1,"\n");
        }

        // cleanup
        close(fd);
    }

    return 0;
}