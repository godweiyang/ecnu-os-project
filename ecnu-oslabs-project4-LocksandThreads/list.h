#ifndef _LIST_H
#define _LIST_H

#include "lock.h"

typedef struct __node_t {
	int key;
	struct __node_t* next;
} node_t;

typedef struct __list_t {
	node_t *head;
	#if LOCK_TYPE == SPIN_LOCK
		spinlock_t lock;
	#elif LOCK_TYPE ==  MUTEX_LOCK
		mutex_t lock;
	#elif LOCK_TYPE == PTHREAD_LOCK
		pthread_mutex_t lock;
	#else
		two_phase_t lock;
	#endif
} list_t;

void list_init(list_t *list);
void list_insert(list_t *list, unsigned int key);
void list_delete(list_t *list, unsigned int key);
void *list_lookup(list_t *list, unsigned int key);
int list_size(list_t *list);
void list_free(list_t *list);

#endif