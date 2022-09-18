#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L 
#define BUFFER_DIM      64
#define SOCKET_NAME     "socket_name"
#define MAX_FILES       "max_file_number"
#define MAX_STORAGE     "max_storage_size"
#define THREAD_NUMBERS  "threads_number"
#define LOG_PATH        "logger_path"

void logs(char * format, ... );

int parse(
    char* configpath,  
    char** socket_name, 
    int* max_file_number, 
    int* max_storage_size, 
    int* threads_number,
    char** logpath
);

#endif