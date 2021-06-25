#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <time.h>
#include "spinlock.h"
#include "mutex.h"
#include "two_phase.h"
#include "lock.h"

void lock_init(void* lock) {
	#if LOCK_TYPE == SPIN_LOCK
		spinlock_init( (spinlock_t*) lock);
	#elif LOCK_TYPE ==  MUTEX_LOCK
		mutex_init( (mutex_t*) lock);
	#elif LOCK_TYPE == PTHREAD_LOCK
		pthread_mutex_init( (pthread_mutex_t*) lock, NULL);
	#else
		two_phase_init( (two_phase_t*) lock);
	#endif
}

void lock_acquire(void *lock) {
	#if LOCK_TYPE == SPIN_LOCK
		spinlock_acquire( (spinlock_t*) lock);
	#elif LOCK_TYPE ==  MUTEX_LOCK
		mutex_acquire( (mutex_t*) lock);
	#elif LOCK_TYPE == PTHREAD_LOCK
		pthread_mutex_lock( (pthread_mutex_t*) lock);
	#else
		two_phase_acquire( (two_phase_t*) lock);
	#endif
}

void lock_release(void *lock) {
	#if LOCK_TYPE == SPIN_LOCK
		spinlock_release( (spinlock_t*) lock);
	#elif LOCK_TYPE ==  MUTEX_LOCK
		mutex_release( (mutex_t*) lock);
	#elif LOCK_TYPE == PTHREAD_LOCK
		pthread_mutex_unlock( (pthread_mutex_t*) lock);
	#else
		two_phase_release( (two_phase_t*) lock);
	#endif
}
