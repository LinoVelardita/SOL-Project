#ifndef CONNECTION_H
#define CONNECTION_H
#define _POSIX_C_SOURCE 200809L 

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

#define UNIX_PATH_MAX 108
#define MAX_CONNECTION_QUEUE 1024

int updatemax(fd_set set, int fdmax);

int init_server(char * sck_name);

int find_max(int a, int b , int c, int d, int e);

#endif /* _MY_CONNECTION_H */