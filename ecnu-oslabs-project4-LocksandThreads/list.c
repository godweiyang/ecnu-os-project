#include "lock.h"
#include "list.h"

void list_init(list_t *list) {
	list->head = NULL;
	lock_init((void*) &list->lock);
}

void list_insert(list_t *list, unsigned int key) {
	lock_acquire((void*) &list->lock);
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	if (new_node == NULL) {
		perror("malloc");
		lock_release((void*) &list->lock);
	}
	new_node->key = key;
	new_node->next = list->head;
	list->head = new_node;
	lock_release((void*) &list->lock);
}

void list_delete(list_t *list, unsigned int key) {
	if (list->head == NULL)
		return;
	lock_acquire((void*) &list->lock);
	node_t *index = list->head, *pre = NULL;
	while (index) {
		if (index->key == key) {
			break;
		}
		pre = index;
		index = index->next;
	}
	if (index) {
		if (pre == NULL) {
			list->head = list->head->next;
			free(index);
		}
		else {
			pre->next = index->next;
			free(index);
		}
	}
	lock_release((void*) &list->lock);
}

void *list_lookup(list_t *list, unsigned int key) {
	if (list->head == NULL)
		return NULL;
	lock_acquire((void*) &list->lock);
	node_t *index = list->head;
	while (index) {
		if (index->key == key) {
			lock_release((void*) &list->lock);
			return (void*)index;
		}
		index = index->next;
	}
	lock_release((void*) &list->lock);
	return NULL;
}

int list_size(list_t* list) {
	node_t *index = list->head;
	int cnt = 0;
	while (index) {
		++cnt;
		index = index->next;
	}
	return cnt;
}

void list_free(list_t *list) {
	if (list->head == NULL) {
		return;
	}
	node_t *index = list->head;
	while (index) {
		node_t *temp = index;
		index = index->next;
		free(temp);
	}
}