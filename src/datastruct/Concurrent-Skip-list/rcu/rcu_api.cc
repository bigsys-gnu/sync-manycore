#include "rcu_api.hh"
#include "new-urcu.h"
#include <atomic>

using namespace rcu_api;

static std::atomic<int> id_gen{0};

class rcu_handle
{
  int id_;
public:
  rcu_handle();
  rcu_handle(bool);             // for real initialization
  void operator = (rcu_handle&& o);
  ~rcu_handle();

  int get_id()
  {
    return id_;
  }
};

thread_local rcu_handle handle;

rcu_handle::rcu_handle()
{
}

rcu_handle::rcu_handle(bool):
  id_{id_gen.fetch_add(1, std::memory_order_relaxed)}
{
  urcu_register(id_);
}

void rcu_handle::operator=(rcu_handle &&o)
{
  id_ = o.id_;
  o.id_ = -1;
}

rcu_handle::~rcu_handle()
{
  if (id_ >= 0)
    {
      urcu_unregister();
    }
}

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
  urcu_writer_lock(0);
}

writer_scope::~writer_scope()
{
  urcu_writer_unlock(0);
}

void rcu_api::init(int thread_num)
{
  urcu_init(thread_num);
}

void rcu_api::regist()
{
  handle = rcu_handle(true);
}

void rcu_api::free(void *ptr)
{
  urcu_free(ptr);
}

void rcu_api::synchronize()
{
  urcu_synchronize();
}
