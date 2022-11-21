#pragma once

#include "mvrlu.h"
#include <cstddef>

#define MVRLU_NEW_DELETE(CLASS_NAME)                                    \
  static void*                                                          \
  operator new(unsigned long nbytes, const std::nothrow_t&) noexcept {  \
    return mvrlu::mvrlu_alloc<CLASS_NAME>();                            \
  }                                                                     \
  static void*                                                          \
  operator new(unsigned long nbytes) {                                  \
    void *p = CLASS_NAME::operator new(nbytes, std::nothrow);           \
    if (p == nullptr)                                                   \
      throw_bad_alloc();                                                \
    return p;                                                           \
  }                                                                     \
  static void                                                           \
  operator delete(void *p, const std::nothrow_t&) noexcept {            \
    mvrlu::mvrlu_free(p);                                               \
  }                                                                     \
                                                                        \
  static void                                                           \
  operator delete(void *p) {                                            \
    CLASS_NAME::operator delete(p, std::nothrow);                       \
  }

namespace mvrlu_api
{
  class thread_handle;

  inline void free(void *ptr)
  {
    ::mvrlu_free(NULL, ptr);
  }

  // call RLU_INIT, RLU_FINISH manually
  // this mvrlu can be applied only one data structure.
  template <typename T>
  void assign_pointer(T **p_ptr, T *obj)
  {
    ::_mvrlu_assign_pointer((void **) p_ptr, (void *) obj);
  }

  template <typename T>
  bool cmp_ptrs(T *obj, T *obj2)
  {
    return ::mvrlu_cmp_ptrs((void *) obj, (void *) obj2);
  }

  template <typename T>
  T *alloc(void)
  {
    return (T *) ::mvrlu_alloc(sizeof(T));
  }

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

  public:
    thread_handle(void);

    ~thread_handle(void);

    template <typename T>
    bool try_lock(T **p_p_obj)
    {
      if(!*p_p_obj)
        return true;
      return ::_mvrlu_try_lock(self_, (void **) p_p_obj, sizeof(T));
    }

    template <typename T>
    bool try_lock_const(T *obj)
    {
      if(!obj)
        return true;

      return ::_mvrlu_try_lock_const(self_, (void *) obj, sizeof(T));
    }

    void mvrlu_abort(void)
    {
      ::mvrlu_abort(self_);
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

  class session
  {
    thread_handle& handle_;
    bool abort_{false};
  public:
    session(thread_handle& handle): handle_(handle)
    {
      handle_.mvrlu_reader_lock();
    }

    void abort()
    {
      abort_ = true;
      handle_.mvrlu_abort();
    }

    ~session()
    {
      if (!abort_)
        {
          handle_.mvrlu_reader_unlock();
        }
    }
  };

  inline void init()
  {
    ::mvrlu_init();
  }

  inline void finish()
  {
    ::mvrlu_finish();
  }

} // namespace mvrlu
