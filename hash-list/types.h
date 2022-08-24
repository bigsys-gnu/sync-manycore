#ifndef _TYPES_H_
#define _TYPES_H_
#include <limits.h>
#include <pthread.h>
/////////////////////////////////////////////////////////
// DEFINES
/////////////////////////////////////////////////////////
#define LIST_VAL_MIN (INT_MIN)
#define LIST_VAL_MAX (INT_MAX)
/////////////////////////////////////////////////////////
// TYPES
/////////////////////////////////////////////////////////
#define NODE_PADDING (16)
#define MAX_BUCKETS (20000)
typedef int val_t;

typedef struct node {
	val_t val;
	struct node *p_next;

	long padding[NODE_PADDING];
} node_t;

typedef struct list {
	node_t *p_head;
	pthread_spinlock_t lock;
} list_t;

typedef struct hash_list {
	int n_buckets;
	list_t *buckets[MAX_BUCKETS];  
} hash_list_t;

#endif
