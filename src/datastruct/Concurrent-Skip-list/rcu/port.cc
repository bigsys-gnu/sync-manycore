#include "node.hh"
#include "port.h"

void port_free(void *ptr)
{
  auto actual_ptr = reinterpret_cast<node_t *>(ptr);
  delete actual_ptr;
}
