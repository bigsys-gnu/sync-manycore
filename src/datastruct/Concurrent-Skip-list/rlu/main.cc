#include "mvrlu_api.hh"
#include "skip_list.hh"
#include <thread>


int main(int argc, char *argv[])
{
  mvrlu_api::init();



  std::thread th([]()
  {
    mvrlu_api::thread_handle handle;
    {
      mvrlu_api::session s(handle);
    }
  });

  th.join();

  mvrlu_api::finish();

  return 0;
}
