#include "port_cxx.hh"
#include "node.hh"

void *port_alloc_x(size_t, unsigned int)
{
  return new node_t();
}

void port_free(void *ptr)
{
  if (ptr != nullptr)
    {
      auto np = reinterpret_cast<node_ptr>(ptr);
      delete np;
    }
}
