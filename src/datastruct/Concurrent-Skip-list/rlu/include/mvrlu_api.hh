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
    return mvrlu_api::alloc<CLASS_NAME>();                              \
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

  // never use these functions directly, instead use derefered pointer class
  void __obj_free(void *ptr);
  bool __try_lock(void **p_p_obj, size_t size);
  bool __try_lock_const(void *obj, size_t size);
  void *__deref(void *master_node_ptr);

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
      self_ = reinterpret_cast<T*>(__deref(master_node_ptr));
    }

    derefered_ptr(derefered_ptr& ptr)
    {
      self_ = ptr.self_;
    }

    derefered_ptr operator = (derefered_ptr& o)
    {
      self_ = o.self_;
      return { *this };
    }

    derefered_ptr operator = (T* master_node_ptr) // use this carefully
    {
      self_ = reinterpret_cast<T*>(__deref(master_node_ptr));
      return { *this };
    }

    bool operator == (const derefered_ptr& o) const
    {
      return ::mvrlu_cmp_ptrs(self_, o.self_);
    }

    bool operator != (const derefered_ptr& o) const
    {
      return !(*this == o);
    }

    bool try_lock()
    {
      return __try_lock(reinterpret_cast<void **>(&self_), sizeof(T));
    }

    bool try_lock_const()
    {
      return __try_lock_const(reinterpret_cast<void *>(self_), sizeof(T));
    }

    T& operator* ()
    {
      return *self_;
    }

    T* operator-> ()
    {
      return self_;
    }

    void free()
    {
      __obj_free(reinterpret_cast<void *>(self_));
      self_ = nullptr;
    }
  };

  std::thread create_thread(const std::function<void()>&& worker);

  template <typename T>
  void *alloc()
  {
    return ::mvrlu_alloc(sizeof(T));
  }

  class session
  {
    bool abort_{false};
  public:
    session();
    void abort();
    ~session();
  };

  class system
  {
  public:
    system()
    {
      ::mvrlu_init();
    }

    ~system()
    {
      ::mvrlu_finish();
    }
  };
} // namespace mvrlu

#endif /* MVRLU_API_H */
