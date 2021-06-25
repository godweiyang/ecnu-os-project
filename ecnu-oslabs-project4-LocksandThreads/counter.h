#ifndef _COUNTER_H
#define _COUNTER_H

#include "lock.h"

typedef struct __counter_t {
	int val;
	#if LOCK_TYPE == SPIN_LOCK
		spinlock_t lock;
	#elif LOCK_TYPE ==  MUTEX_LOCK
		mutex_t lock;
	#elif LOCK_TYPE == PTHREAD_LOCK
		pthread_mutex_t lock;
	#else
		two_phase_t lock;
	#endif
} counter_t;

void counter_init(counter_t *c, int val);
int counter_get_value(counter_t *c);
void counter_increment(counter_t *c);
void counter_decrement(counter_t *c);

#endif