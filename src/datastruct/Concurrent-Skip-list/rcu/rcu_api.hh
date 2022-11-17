#pragma once

#include <thread>
#include <utility>
#include "new-urcu.h"

namespace rcu_api
{
  class rcu_handle
  {
    int id_;
  public:
    rcu_handle():
      id_(std::hash<std::thread::id>{}(std::this_thread::get_id()))
    {
      urcu_register(id_);
    }

    ~rcu_handle()
    {
      urcu_unregister();
    }

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
