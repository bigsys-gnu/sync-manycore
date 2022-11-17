#include "rcu_api.hh"
#include "new-urcu.h"
#include <atomic>

using namespace rcu_api;

thread_local rcu_handle handle;
thread_local unsigned int reader_nested = 0;

static std::atomic<int> id_gen{0};

rcu_handle::rcu_handle():
  id_{id_gen.fetch_add(1, std::memory_order_relaxed)}
{
  urcu_register(id_);
}

rcu_handle::~rcu_handle()
{
  urcu_unregister();
}

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

void rcu_api::synchronize()
{
  urcu_synchronize();
}
