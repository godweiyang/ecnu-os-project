#include "lock.h"
#include "counter.h"

void counter_init(counter_t *c, int val) {
	c->val = val;
	lock_init((void*) &c->lock);
}

int counter_get_value(counter_t *c) {
	lock_acquire((void*) &c->lock);
	int val = c->val;
	lock_release((void*) &c->lock);
	return val;
}

void counter_increment(counter_t *c) {
	lock_acquire((void*) &c->lock);
	c->val++;
	lock_release((void*) &c->lock);
}

void counter_decrement(counter_t *c) {
	lock_acquire((void*) &c->lock);
	c->val--;
	lock_release((void*) &c->lock);
}
