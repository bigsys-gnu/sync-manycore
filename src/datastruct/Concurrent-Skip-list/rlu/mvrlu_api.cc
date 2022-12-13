#include <memory>
#include "mvrlu_api.hh"

using namespace mvrlu_api;

thread_handle::thread_handle()
{
  self_ = nullptr;
  if(::mvrlu_is_init() != 0)
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

void thread_handle::operator=(thread_handle &&o)
{
  self_ = o.self_;
  o.self_ = nullptr;
}

std::thread
mvrlu_api::create_thread(const std::function<void ()> &&worker)
{
  return std::thread([worker=std::move(worker)]()
  {
    auto ptr = std::make_unique<thread_handle>();
    handle = ptr.get();
    worker();
  });
}
