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

  void free(void* ptr);

  void synchronize();
}
