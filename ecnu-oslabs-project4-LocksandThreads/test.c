#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/time.h>

#include "lock.h"
#include "counter.h"
#include "list.h"
#include "hash.h"

#define COUNTER_T 0
#define LIST_T 1
#define HASH_T 2

#define TEST_TYPE 2

#if TEST_TYPE == COUNTER_T
	counter_t *t;
#elif TEST_TYPE == LIST_T
	list_t *t;
#else
	hash_t *t;
#endif

pthread_t *p;

void test(void *arg) {
	int val = *(int*)arg;
	while (val--) {
		#if TEST_TYPE == COUNTER_T
			counter_increment(t);
		#elif TEST_TYPE == LIST_T
			list_insert(t, val);
		#else
			hash_insert(t, val);
		#endif
	}
}

long getCurrentTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void check_and_release() {
	#if TEST_TYPE == COUNTER_T
		//    printf("%d\n", counter_get_value(t));
	#endif
	
	#if LOCK_TYPE == PTHREAD_LOCK && TEST_TYPE != HASH_T
		pthread_mutex_destroy(&t->lock);
	#endif
	
	#if LOCK_TYPE == PTHREAD_LOCK && TEST_TYPE == HASH_T
		list_t *index = t->list_head;
		int i;
		for (i = 0; i < t->size; ++i) {
			list_t *cur = index + i;
			pthread_mutex_destroy(&cur->lock);
		}
	#endif
		
	#if TEST_TYPE == HASH_T
		// printf("%d\n", hash_total_size(t));
		hash_free(t);
	#endif
	
	#if TEST_TYPE == LIST_T
		// printf("%d\n", list_size(t));
		list_free(t);
	#endif
	
	free(p);
}

int main() {
	#if TEST_TYPE == COUNTER_T
		freopen("counter.txt", "a", stdout);
	#elif TEST_TYPE == LIST_T
		freopen("list.txt", "a", stdout);
	#else
		freopen("hash.txt", "a", stdout);
	#endif

	#if TEST_TYPE == COUNTER_T
		printf("counter\n");
	#elif TEST_TYPE == LIST_T
		printf("list\n");
	#else
		printf("hash\n");
	#endif

	#if LOCK_TYPE == SPIN_LOCK
		printf("spinlock\n");
	#elif LOCK_TYPE ==  MUTEX_LOCK
		printf("mutex\n");
	#elif LOCK_TYPE == PTHREAD_LOCK
		printf("pthread_lock\n");
	#else
		printf("two_phase\n");
	#endif
	
	int number = 1000;
	int thread_no = 1;

	for (number = 1000; number <= 1000000; number *= 10) {
		printf("number: %d\n", number);
		for (thread_no = 1; thread_no <= 20; ++thread_no) {
			long start_time = getCurrentTime();
	
			#if TEST_TYPE == COUNTER_T
				t = (counter_t*)malloc(sizeof(counter_t));
				counter_init(t, 0);
			#elif TEST_TYPE == LIST_T
				t = (list_t*)malloc(sizeof(list_t));
				list_init(t);
			#else
				t = (hash_t*)malloc(sizeof(hash_t));
				hash_init(t, 30);
			#endif
			
			int i;
			p = (pthread_t*)malloc(sizeof(pthread_t) * thread_no);
			
			for (i = 0; i < thread_no; ++i) {
				pthread_create(p+i, NULL, (void*)test, &number);
			}
			
			for (i = 0; i < thread_no; ++i) {
				pthread_join(p[i], NULL);
				// printf("%d completed\n", i);
			}
			

			printf("%.3lf ", (double)(getCurrentTime() - start_time) / 1000.0);
			
			check_and_release();
		}
		puts("");
	}
	
	

	return 0;
}
