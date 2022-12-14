#ifndef _MY_HANDLER_H
#define _MY_HANDLER_H
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

extern int pipeSigWriting;

void ter_handler(int sig);

void handler_installer();
void worker_handler_installer();
void print_handler(int sig);

#endif /* _MY_HANDLER_H */