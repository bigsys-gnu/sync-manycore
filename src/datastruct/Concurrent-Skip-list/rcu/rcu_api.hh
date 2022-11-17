#pragma once

#include <thread>
#include <utility>
#include <iostream>
#include "new-urcu.h"

namespace rcu_api
{

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

  void regist();

  template <typename T>
  void free(T* ptr)
  {
    urcu_free(reinterpret_cast<void *>(ptr));
  }

  void synchronize();
}
