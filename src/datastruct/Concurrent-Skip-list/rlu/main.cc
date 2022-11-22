#include "mvrlu_api.hh"
#include "node.hh"
#include "skip_list.hh"
#include <thread>
#include <iostream>

int main(int argc, char *argv[])
{
  mvrlu_api::system mvrlu_system;

  auto th = mvrlu_api::create_thread([]()
  {
    std::cout << "hello\n";
    {
      mvrlu_api::session se;
    }
  });

  th.join();

  return 0;
}
