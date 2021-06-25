#include "two_phase.h"
#include "sys_futex.c"
#include "xchg.c"

void two_phase_init(two_phase_t *lock) {
	lock->flag = 0;
	lock->count = 0;
	lock->guard_acquire = 0;
}

void two_phase_acquire(two_phase_t *lock) {
	if (xchg(&(lock->flag), 1) == 0) {
		return;
	}

	while (xchg(&(lock->guard_acquire), 1) == 1);
	lock->count++;
	lock->guard_acquire = 0;
	
	while (1) {
		if (xchg(&(lock->flag), 1) == 0) {
			while (xchg(&(lock->guard_acquire), 1) == 1);
			lock->count--;
			lock->guard_acquire = 0;
			break;
		}
		
		int v = lock->flag;
		
		if (lock->flag == 0) {
			continue;
		}
		sys_futex(&(lock->flag), FUTEX_WAIT, v, 0, 0, 0);
	}
	return;
}

void two_phase_release(two_phase_t *lock) {
	lock->flag = 0;
	if(lock->count != 0) {
		sys_futex(&(lock->flag), FUTEX_WAKE, lock->count, 0, 0, 0);
	}
}