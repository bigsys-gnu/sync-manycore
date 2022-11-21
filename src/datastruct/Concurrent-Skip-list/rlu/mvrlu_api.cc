#include "mvrlu_api.hh"
#include "mvrlu_i.h"

using namespace mvrlu_api;

thread_handle::thread_handle()
{
  self_ = nullptr;
  if(::mvrlu_is_init())
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

thread_local thread_handle handle;

void mvrlu_api::regist()
{
  handle = thread_handle();
}
