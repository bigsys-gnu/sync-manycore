#ifndef _HASH_LIST_H_
#define _HASH_LIST_H_

/////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////
#include "types.h"


/////////////////////////////////////////////////////////
// INTERFACE
/////////////////////////////////////////////////////////
hash_list_t *pure_new_hash_list(int n_buckets);

int hash_list_size(hash_list_t *p_hash_list);
void hash_list_print(hash_list_t *p_hash_list);

int pure_hash_list_contains(hash_list_t *p_hash_list, val_t val);
int pure_hash_list_add(hash_list_t *p_hash_list, val_t val);
int pure_hash_list_remove(hash_list_t *p_hash_list, val_t val);

#endif // _HASH_LIST_H_
