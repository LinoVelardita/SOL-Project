#ifndef _MYSERVERAPI_H
#define _MYSERVERAPI_H

#define _POSIX_C_SOURCE 200809L 
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

//myperror
#define END_RET_FILE -1
#define SUCCESS 0
#define RET_FILE 1  
#define E_INV_FLG 140 
#define E_INV_PTH 141 
#define E_LOCK 142    
#define E_NOT_EX 143  
#define E_ALR_EX 144      
#define E_BAD_RQ 145  
#define E_ALR_LK 146
#define E_NO_SPACE 147
#define E_NOT_OPN 148 
#define E_INV_SCK 149 
#define E_NOT_CON 150


//flags per l'openFile
#define O_CREATE 1
#define O_LOCK 2

#define MAX_PATH 1024
#define UNIX_PATH_MAX 108
#define BUF_SIZE 2048

//requestType
#define OPEN_F 1
#define READ_F 2
#define READ_N_F 3
#define WRITE_F 4
#define APPEND_T_F 5
#define LOCK_F 6
#define UNLOCK_F 7
#define CLOSE_F 8
#define REMOVE_F 9

int openConnection(const char* sockname, int msec, const struct timespec abstime);
int closeConnection(const char* sockname);
int openFile(const char* pathname, int flags);
int writeFile(char* pathname, char * dirname);
int appendToFile(char * pathname, void * buf, size_t size, char * dirname);
int lockFile(char * pathname);
int unlockFile(char * pathname);
int closeFile(char * pathname);
int removeFile(char * pathname);
int readFile(char * pathname, void ** buf, size_t * size);
void myperror(const char * str);
int readNFiles(int n, char * dirname);
int onlyName(char * str);
void lock_file(char * str);
void unlock_file(char * str);
void remove_file(char * str);
int readn(int fd, void *ptr, size_t n);
int writen(int fd, void *ptr, size_t n);
void ms2ts(struct timespec *ts, unsigned long ms);
void timespec_diff(struct timespec *a, struct timespec *b, struct timespec *result);
void msleep(int msec);

#define PIE(exp) \
        if( (exp) == -1){ \
            myperror(#exp);\
        }\
    
#endif /*_MYSERVERAPI_H*/ 
