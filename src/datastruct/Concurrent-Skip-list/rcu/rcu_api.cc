#include "rcu_api.hh"
#include "new-urcu.h"

using namespace rcu_api;

thread_local rcu_handle handle;
thread_local unsigned int reader_nested = 0;

reader_scope::reader_scope()
{
  if (reader_nested == 0)
    {
      urcu_reader_lock();
    }
  reader_nested++;
}

reader_scope::~reader_scope()
{
  reader_nested--;
  if (reader_nested == 0)
    {
      urcu_reader_unlock();
    }
}

writer_scope::writer_scope()
{
  urcu_writer_lock(handle.get_id());
}

writer_scope::~writer_scope()
{
  urcu_writer_unlock(handle.get_id());
}

void synchronize()
{
  urcu_synchronize();
}
