#pragma once

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
    mvrlu_api::free(p);                                                 \
  }                                                                     \
                                                                        \
  static void                                                           \
  operator delete(void *p) {                                            \
    CLASS_NAME::operator delete(p, std::nothrow);                       \
  }

namespace mvrlu_api
{
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

  std::thread create_thread(const std::function<void()>&& worker);
} // namespace mvrlu
