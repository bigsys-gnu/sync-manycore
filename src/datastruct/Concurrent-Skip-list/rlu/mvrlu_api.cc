#include "mvrlu_api.hh"
#include "mvrlu_i.h"

namespace mvrlu_api
{
  void __obj_free(void *ptr);
  bool __try_lock(void **p_p_obj, size_t size);
  bool __try_lock_const(void *obj, size_t size);
  void *__deref(void *master_node_ptr);
  class thread_handle
  {
    friend class session;

    void mvrlu_reader_lock(void)
    {
      ::mvrlu_reader_lock(self_);
    }

    void mvrlu_reader_unlock(void)
    {
      ::mvrlu_reader_unlock(self_);
    }

    void mvrlu_abort(void)
    {
      ::mvrlu_abort(self_);
    }

  public:
    thread_handle(void);

    ~thread_handle(void);

    void operator=(thread_handle&&);

    bool try_lock(void **p_p_obj, size_t size)
    {
      if(!*p_p_obj)
        return true;
      return ::_mvrlu_try_lock(self_, (void **) p_p_obj, size);
    }

    bool try_lock_const(void *obj, size_t size)
    {
      if(!obj)
        return true;

      return ::_mvrlu_try_lock_const(self_, (void *) obj, size);
    }

    template <typename T>
    T *mvrlu_deref(T *p_obj)
    {
      return (T *) ::mvrlu_deref(self_, (void *) p_obj);
    }

    // need hotfix!!!
    // - how to call deleter of p_obj properly
    // 1. mvrlu.c don't know p_obj is which type.
    // -> mvrlu.c need to be fixed using template?
    template <typename T>
    void mvrlu_free(T *p_obj)
    {
      ::mvrlu_free(self_, (void *) p_obj);
    }

    void mvrlu_flush_log(void)
    {
      ::mvrlu_flush_log(self_);
    }

  private:
    mvrlu_thread_struct_t *self_;
  };
}


using namespace mvrlu_api;

thread_handle::thread_handle()
{
  self_ = nullptr;
  if(::mvrlu_is_init() != 0)
    {
      self_ = ::mvrlu_thread_alloc();
      ::mvrlu_thread_init(self_);
    }
}

thread_handle::~thread_handle(void)
{
  if(self_)
    {
      ::mvrlu_thread_finish(self_);
      ::mvrlu_thread_free(self_);
    }
}

void thread_handle::operator=(thread_handle &&o)
{
  self_ = o.self_;
  o.self_ = nullptr;
}

thread_local thread_handle handle;

std::thread
mvrlu_api::create_thread(const std::function<void ()> &&worker)
{
  return std::thread([&]()
  {
    handle = thread_handle();
    worker();
  });
}

session::session()
{
  handle.mvrlu_reader_lock();
}

void session::abort()
{
  abort_ = true;
  handle.mvrlu_abort();
}

session::~session()
{
  if (!abort_)
    {
      handle.mvrlu_reader_unlock();
    }
}

void mvrlu_api::__obj_free(void *ptr)
{
  handle.mvrlu_free(ptr);
}

bool mvrlu_api::__try_lock(void **p_p_obj, size_t size)
{
  return handle.try_lock(p_p_obj, size);
}

bool mvrlu_api::__try_lock_const(void *obj, size_t size)
{
  return handle.try_lock_const(obj, size);
}

void * mvrlu_api::__deref(void *master_node_ptr)
{
  return handle.mvrlu_deref(master_node_ptr);
}
