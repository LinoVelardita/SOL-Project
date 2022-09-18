#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

/***** Shared Queue   ****/
#define SHARED_QUEUE_MAX_DIM 2048

typedef struct {
    int* set;
    int start;
    int current_size;

    pthread_mutex_t lock;
    pthread_cond_t is_full;
    pthread_cond_t is_empty;
} SharedQueue_t;

SharedQueue_t * SharedQueue();
void push(SharedQueue_t * q, int fd_c);
int pop(SharedQueue_t * q);


/***** Linked List   ****/

typedef struct node {
	int data;
	struct node* next;
} List_t;

void List(List_t** list, int data);
void insert_l(List_t** list, int data);
void remove_l(List_t** list, int data);
void destroy_l(List_t** list);
int mem_l(List_t** list, int data);

#endif
