#include "datastructures.h"

SharedQueue_t* SharedQueue(){
    SharedQueue_t* q;

    ASSERT_EXIT( q = (SharedQueue_t*)malloc(sizeof(SharedQueue_t)), == NULL);
    ASSERT_EXIT( memset(q, 0, sizeof(SharedQueue_t)), == NULL);
    ASSERT_EXIT(q->set = malloc(sizeof(int) * SHARED_QUEUE_MAX_DIM), == NULL);
    ASSERT_EXIT(pthread_mutex_init(&q->lock, NULL), != 0);
    ASSERT_EXIT(pthread_cond_init(&q->is_full, NULL), != 0);
    ASSERT_EXIT(pthread_cond_init(&q->is_empty, NULL), != 0);

    q->start = 0;
    q->current_size = 0;

    return q;
}

void push(SharedQueue_t * q, int fd_c){
    ASSERT_EXIT(q, == NULL);
    ASSERT_EXIT(pthread_mutex_lock(&q->lock), != 0);	//lock sulla coda

    while(q->current_size >= SHARED_QUEUE_MAX_DIM){	//se la coda è piena aspetto (wait)
        ASSERT_EXIT(pthread_cond_wait(&q->is_full, &q->lock), != 0);
    }

    q->set[ (q->start + q->current_size) % SHARED_QUEUE_MAX_DIM] = fd_c;
    q->current_size++;

    ASSERT_EXIT(pthread_cond_signal(&q->is_empty), != 0);
    ASSERT_EXIT(pthread_mutex_unlock(&q->lock), != 0);
}

int pop(SharedQueue_t * q){
    ASSERT_EXIT(q, == NULL);
    ASSERT_EXIT(pthread_mutex_lock(&q->lock), != 0);

    while(q->current_size <= 0){
        ASSERT_EXIT(pthread_cond_wait(&q->is_empty, &q->lock), != 0);
    }

    int ret = q->set[ q->start % SHARED_QUEUE_MAX_DIM];
    q->start++;
    q->current_size--;
    q->start = q->start % SHARED_QUEUE_MAX_DIM;

    ASSERT_EXIT(pthread_cond_signal(&q->is_full), != 0);
    ASSERT_EXIT(pthread_mutex_unlock(&q->lock), != 0);

    return ret;
}

void List(List_t** list, int data){
	ASSERT_EXIT(*list = (List_t*) malloc(sizeof(List_t)), == NULL);

	(*list)->data = data;
	(*list)->next = NULL;

	return;
}

void insert_l(List_t** list, int data){
    if (*list == NULL){
        List(list,data);
		return;
    }

	List_t* current = *list;
	List_t* tmp;

	while(current){
		if(current->data == data) return;
		tmp = current;
		current = current->next;
	} 

	List_t* new_node;
    ASSERT_EXIT(new_node = (List_t*) malloc(sizeof(List_t)), == NULL);
	
	new_node->next = NULL;
	new_node->data = data;
	tmp->next = new_node;
	return;
}

void remove_l(List_t** list, int data){
	if(!mem_l(list, data)) 
        return;
	
	List_t* current = *list;
	List_t* prev = NULL;

	do {
		if (current->data == data) {
			break;
		}
		prev = current;
		current = current->next;
	} while (current);

	if (current == *list) {
		prev = *list;
		*list = current->next;
		free(prev);
		return;
	}

	if (current->next == NULL) {
		prev->next = NULL;
		free(current);
		return;
	}

	prev->next = current->next;
	free(current);
	return;
}

void destroy_l(List_t** list){
	List_t* node = *list;
	while (node) {
		List_t* tmp;
		tmp = node;
		node = node->next;
		free(tmp);
	}
}

int mem_l(List_t** list, int data){
    if (*list == NULL){
        return 0;
    }

    List_t* current = *list;

	while (current){
        if (current->data == data){
            return 1;
        }
		current = current->next;
	} 

	return 0;
}
