#include "port_cxx.hh"
#include <cstdlib>

void *port_alloc_x(size_t s, unsigned int)
{
  return malloc(s);
}

void port_free(void *ptr)
{
  if (ptr != nullptr)
    {
      free(ptr);
    }
}
