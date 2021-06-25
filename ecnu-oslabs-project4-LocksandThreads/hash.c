#include "lock.h"
#include "list.h"
#include "hash.h"

void hash_init(hash_t *hash, int size) {
	hash->size = size;
	hash->list_head = (list_t*) malloc(sizeof(list_t) * size);
	int i;
	for (i = 0; i < size; ++i) {
		list_init(hash->list_head + i);
	}
}

void hash_insert(hash_t *hash, unsigned int key) {
	int bucket = key % hash->size;
	list_insert(hash->list_head + bucket, key);
}

void hash_delete(hash_t *hash, unsigned int key) {
	int bucket = key % hash->size;
	list_delete(hash->list_head + bucket, key);
}

void *hash_lookup(hash_t *hash, unsigned int key) {
	int bucket = key % hash->size;
	return list_lookup(hash->list_head + bucket, key);
}

int hash_total_size(hash_t *hash) {
	int i, cnt = 0;
	for (i = 0; i < hash->size; ++i) {
		cnt += list_size(hash->list_head + i);
	}
	return cnt;
}

void hash_free(hash_t *hash) {
	int i;
	for (i = 0; i < hash->size; ++i) {
		list_free(hash->list_head + i);
	}
}