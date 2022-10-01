
/////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include <pthread.h>

/////////////////////////////////////////////////////////
// DEFINES
/////////////////////////////////////////////////////////
#define HASH_VALUE(p_hash_list, val)       (val % p_hash_list->n_buckets)

#define MEMBARSTLD() __sync_synchronize()
#ifndef CAS
#define CAS(addr, expected_value, new_value) __sync_val_compare_and_swap((addr), (expected_value), (new_value))
#endif

/////////////////////////////////////////////////////////
// NEW NODE
/////////////////////////////////////////////////////////
node_t *pure_new_node() {

	node_t *p_new_node = (node_t *)malloc(sizeof(node_t));
	if (p_new_node == NULL){
		printf("out of memory\n");
		exit(1);
	}

    return p_new_node;
}

/////////////////////////////////////////////////////////
// FREE NODE
/////////////////////////////////////////////////////////
void pure_free_node(node_t *p_node) {
	if (p_node != NULL) {
		free(p_node);
	}
}


/////////////////////////////////////////////////////////
// NEW LIST
/////////////////////////////////////////////////////////
list_t *pure_new_list()
{
	list_t *p_list;
	node_t *p_min_node, *p_max_node;


	p_list = (list_t *)malloc(sizeof(list_t));
	
	 pthread_spin_init(&p_list->lock,0);
	
	if (p_list == NULL) {
		perror("malloc");
		exit(1);
	}

	p_max_node = pure_new_node();
	p_max_node->val = LIST_VAL_MAX;
	p_max_node->p_next = NULL;

	p_min_node = pure_new_node();
	p_min_node->val = LIST_VAL_MIN;
	p_min_node->p_next = p_max_node;

	p_list->p_head = p_min_node;

	return p_list;
}

/////////////////////////////////////////////////////////
// NEW HASH LIST
/////////////////////////////////////////////////////////
hash_list_t *pure_new_hash_list(int n_buckets)
{
	int i;
	hash_list_t *p_hash_list;

	p_hash_list = (hash_list_t *)malloc(sizeof(hash_list_t));

	if (p_hash_list == NULL) {
	    perror("malloc");
	    exit(1);
	}

	p_hash_list->n_buckets = n_buckets;

	for (i = 0; i < p_hash_list->n_buckets; i++) {
		p_hash_list->buckets[i] = pure_new_list();
	}

	return p_hash_list;
}


/////////////////////////////////////////////////////////
// LIST SIZE
/////////////////////////////////////////////////////////
int list_size(list_t *p_list)
{
	int size = 0;
	node_t *p_node;

	/* We have at least 2 elements */
	p_node = p_list->p_head->p_next;
	while (p_node->p_next != NULL) {
		size++;
		p_node = p_node->p_next;
	}

	return size;
}

void list_print(list_t *p_list)
{
	node_t *p_node;

	/* We have at least 2 elements */
	p_node = p_list->p_head->p_next;
	while (p_node->p_next != NULL) {
		printf("%u ", p_node->val);
		p_node = p_node->p_next;
	}
}

/////////////////////////////////////////////////////////
// HASH LIST SIZE
/////////////////////////////////////////////////////////
int hash_list_size(hash_list_t *p_hash_list)
{
	int i;
	int size = 0;

	for (i = 0; i < p_hash_list->n_buckets; i++) {
		size += list_size(p_hash_list->buckets[i]);
	}

	return size;
}

void hash_list_print(hash_list_t *p_hash_list)
{
	int i;

	for (i = 0; i < p_hash_list->n_buckets; i++) {
		list_print(p_hash_list->buckets[i]);
	}
}


/////////////////////////////////////////////////////////
// LIST CONTAINS
/////////////////////////////////////////////////////////
int pure_list_contains(list_t *p_list, val_t val) {
	pthread_spin_lock(&p_list->lock);

	node_t *p_prev, *p_next;

	p_prev = p_list->p_head;
	p_next = p_prev->p_next;
	while (p_next->val < val) {
		p_prev = p_next;
		p_next = p_prev->p_next;
	}
	pthread_spin_unlock(&p_list->lock);
	return p_next->val == val;
}

/////////////////////////////////////////////////////////
// HASH LIST CONTAINS
/////////////////////////////////////////////////////////
int pure_hash_list_contains(hash_list_t *p_hash_list, val_t val)
{
	int hash = HASH_VALUE(p_hash_list, val);

	return pure_list_contains(p_hash_list->buckets[hash], val);
}

/////////////////////////////////////////////////////////
// LIST ADD
/////////////////////////////////////////////////////////
int pure_list_add(list_t *p_list, val_t val)
{
	pthread_spin_lock(&p_list->lock);

	int result;
	node_t *p_prev, *p_next, *p_new_node;

	p_prev = p_list->p_head;
	p_next = p_prev->p_next;
	while (p_next->val < val) {
		p_prev = p_next;
		p_next = p_prev->p_next;
	}

	result = (p_next->val != val);

	if (result) {
		p_new_node = pure_new_node();
		p_new_node->val = val;
		p_new_node->p_next = p_next;

		p_prev->p_next = p_new_node;
	}
	pthread_spin_unlock(&p_list->lock);
	return result;
}


/////////////////////////////////////////////////////////
// HASH LIST ADD
/////////////////////////////////////////////////////////
int pure_hash_list_add(hash_list_t *p_hash_list, val_t val)
{
	int hash = HASH_VALUE(p_hash_list, val);

	return pure_list_add(p_hash_list->buckets[hash], val);
}

/////////////////////////////////////////////////////////
// LIST REMOVE
/////////////////////////////////////////////////////////
int pure_list_remove(list_t *p_list, val_t val) {
	pthread_spin_lock(&p_list->lock);

	int result;
	node_t *p_prev, *p_next;

	p_prev = p_list->p_head;
	p_next = p_prev->p_next;
	while (p_next->val < val) {
		p_prev = p_next;
		p_next = p_prev->p_next;
	}

	result = (p_next->val == val);

	if (result) {
		p_prev->p_next = p_next->p_next;
		pure_free_node(p_next);
	}
	pthread_spin_unlock(&p_list->lock);
	return result;
}


/////////////////////////////////////////////////////////
// HASH LIST REMOVE
/////////////////////////////////////////////////////////
int pure_hash_list_remove(hash_list_t *p_hash_list, val_t val)
{
	int hash = HASH_VALUE(p_hash_list, val);

	return pure_list_remove(p_hash_list->buckets[hash], val);
}
