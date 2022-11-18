#pragma once
#ifndef PORT_CXX_H
#define PORT_CXX_H

/*
 * Memory allocation
 */

#define PORT_DEFAULT_ALLOC_FLAG 0

#ifndef __cplusplus
#include "arch.h"
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *port_alloc_x(size_t, unsigned int);
void port_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* PORT_CXX_H */
