#pragma once

#include <thread>
#include <utility>
#include <iostream>
#include "new-urcu.h"

namespace rcu_api
{
  class rcu_handle
  {
    int id_;
  public:
    rcu_handle();

    ~rcu_handle();

    int get_id()
    {
      return id_;
    }
  };

  class reader_scope
  {
  public:
    reader_scope();
    ~reader_scope();
  };

  class writer_scope
  {
  public:
    writer_scope();
    ~writer_scope();
  };

  template <typename T>
  void free(T* ptr)
  {
    urcu_free(reinterpret_cast<void *>(ptr));
  }

  void synchronize();
}
