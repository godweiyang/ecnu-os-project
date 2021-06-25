#ifndef _LOCK_H
#define _LOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <time.h>
#include "spinlock.h"
#include "mutex.h"
#include "two_phase.h"

#define PTHREAD_LOCK   0
#define SPIN_LOCK      1
#define MUTEX_LOCK     2
#define TWO_PHASE_LOCK 3

#define LOCK_TYPE 3

void lock_init(void* lock);
void lock_acquire(void *lock);
void lock_release(void *lock);

#endif
