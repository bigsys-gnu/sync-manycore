#pragma once

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

  void init(int thread_num);

  void regist();

  void free(void* ptr);

  void synchronize();
}
