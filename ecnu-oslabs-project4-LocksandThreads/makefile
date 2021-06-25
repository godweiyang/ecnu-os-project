all: liblock.so libcounter.so liblist.so libhash.so test
liblock.so: lock.c lock.h spinlock.c spinlock.h mutex.c mutex.h two_phase.c two_phase.h xchg.c sys_futex.c
	@gcc -shared -fPIC -o liblock.so -lpthread lock.c lock.h\
		spinlock.c spinlock.h mutex.c mutex.h two_phase.c two_phase.h xchg.c sys_futex.c
libcounter.so: counter.c counter.h
	@gcc -shared -fPIC -o libcounter.so -I. -L. -llock counter.c counter.h
liblist.so: list.c list.h
	@gcc -shared -fPIC -o liblist.so -I. -L. -llock list.c list.h
libhash.so: hash.c hash.h
	@gcc -shared -fPIC -o libhash.so -I. -L. -llock list.c list.h hash.c hash.h
test: test.c
	@gcc -o test test.c -lpthread -llist -lcounter -lhash -llock -L. -Wall -Werror
clean:
	rm -f liblock.so libcounter.so liblist.so libhash.so test
