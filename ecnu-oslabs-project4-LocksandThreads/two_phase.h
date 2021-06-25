#ifndef _TWO_PHASE_H
#define _TWO_PHASE_H

typedef struct __two_phase_t {
	int flag;
	int count;
	int guard_acquire;
} two_phase_t;

void two_phase_init(two_phase_t *lock);
void two_phase_acquire(two_phase_t *lock);
void two_phase_release(two_phase_t *lock);

#endif
