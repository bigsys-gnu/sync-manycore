#pragma once
#ifndef MVRLU_API_H
#define MVRLU_API_H

#include "mvrlu.h"
#include <thread>
#include <functional>
#include <cstddef>

#define MVRLU_NEW_DELETE(CLASS_NAME)                                    \
  static void*                                                          \
  operator new(unsigned long nbytes, const std::nothrow_t&) noexcept {  \
    return ::mvrlu_alloc(nbytes);                                       \
  }                                                                     \
  static void*                                                          \
  operator new(unsigned long nbytes) {                                  \
    void *p = CLASS_NAME::operator new(nbytes, std::nothrow);           \
    if (p == nullptr)                                                   \
      throw std::bad_alloc();                                           \
    return p;                                                           \
  }                                                                     \
  static void                                                           \
  operator delete(void *p, const std::nothrow_t&) noexcept {            \
    ::mvrlu_free(nullptr, p);                                           \
  }                                                                     \
                                                                        \
  static void                                                           \
  operator delete(void *p) {                                            \
    CLASS_NAME::operator delete(p, std::nothrow);                       \
  }

namespace mvrlu_api
{
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
      return ::_mvrlu_try_lock(self_, (void **) p_p_obj, size);
    }

    bool try_lock_const(void *obj, size_t size)
    {
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
    struct mvrlu_thread_struct *self_;
  };

  inline thread_local thread_handle * handle;

  inline thread_handle& get_handle()
  {
    return *handle;
  }

  template <typename T>
  void assign_pointer(T **p_ptr, T *obj)
  {
    ::_mvrlu_assign_pointer((void **) p_ptr, (void *) obj);
  }

  // use this derefered_ptr only in single session
  template <typename T>
  class derefered_ptr
  {
    T *self_{nullptr}; // dereferenced pointer
  public:
    derefered_ptr()
    {
    }

    derefered_ptr(T* master_node_ptr) // initialize with master node pointer!
    {
      self_ = reinterpret_cast<T*>(get_handle().mvrlu_deref(master_node_ptr));
    }

    derefered_ptr(const derefered_ptr& ptr)
    {
      self_ = ptr.self_;
    }

    derefered_ptr& operator = (const derefered_ptr& o)
    {
      self_ = o.self_;
      return *this;
    }

    derefered_ptr& operator = (T* master_node_ptr) // use this carefully
    {
      self_ = reinterpret_cast<T*>(get_handle().mvrlu_deref(master_node_ptr));
      return *this;
    }

    bool operator == (const derefered_ptr& o) const
    {
      return self_ == o.self_;
    }

    bool operator != (const derefered_ptr& o) const
    {
      return !(*this == o);
    }

    bool try_lock()
    {
      return get_handle().try_lock(reinterpret_cast<void **>(&self_), sizeof(T));
    }

    bool try_lock_const()
    {
      return get_handle().try_lock_const(reinterpret_cast<void *>(self_), sizeof(T));
    }

    T& operator* () const
    {
      return *self_;
    }

    T* operator-> () const
    {
      return self_;
    }

    void free()
    {
      get_handle().mvrlu_free(reinterpret_cast<void *>(self_));
      self_ = nullptr;
    }

    T* get() const
    {
      return self_;
    }

  };

  std::thread create_thread(const std::function<void()>&& worker);

  class session
  {
    bool abort_{false};
  public:
    session()
    {
      get_handle().mvrlu_reader_lock();
    }
    void abort()
    {
      abort_ = true;
      get_handle().mvrlu_abort();
    }
    ~session()
    {
      if (!abort_)
        {
          get_handle().mvrlu_reader_unlock();
        }
    }
  };

  class system
  {
    inline static bool init_{false};
  public:
    system()
    {
      if (!init_)
        {
          ::mvrlu_init();
          init_ = true;
        }
    }

    ~system()
    {
      if (init_)
        {
          ::mvrlu_finish();
          init_ = false;
        }
    }
  };

  inline void print_stats()
  {
    ::mvrlu_print_stats();
  }

} // namespace mvrlu

#endif /* MVRLU_API_H */
