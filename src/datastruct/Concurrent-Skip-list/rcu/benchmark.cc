/**
	Program to perform benchmark testing on the skip list.
*/

#include <iostream>
#include <thread>
#include <random>
#include <condition_variable>
#include <string>
#include "rcu_api.hh"
#include "skip_list.hh"

struct global_data
{
  unsigned int thread_num{8};
  std::default_random_engine engine;
  std::uniform_int_distribution<unsigned int> dist = std::uniform_int_distribution<unsigned int>(1, 3);
  std::uniform_int_distribution<int> key_dist = std::uniform_int_distribution<int>();
  std::mutex cond_lock;
  std::condition_variable condvar;
  bool stop = false;
  SkipList skiplist;
};

void worker(global_data& gd)
{
  rcu_api::regist();
  std::cout << "registered\n";
  std::unique_lock<std::mutex> lk(gd.cond_lock); // hold lock
  std::cout << "sleep for cond\n";
  gd.condvar.wait(lk);
  lk.unlock();
  std::cout << "wait is done\n";

  while (!gd.stop)
    {
      auto op = gd.dist(gd.engine);
      auto data = "hello RCU";
      auto key = gd.key_dist(gd.engine);

      switch (op)
        {
        case 1:
          gd.skiplist.add(key, data);
          break;
        case 2:
          gd.skiplist.remove(key);
          break;
        case 3:
          rcu_api::reader_scope read_session;
          gd.skiplist.search(key);
          break;
        }
    }
}


int main(int argc, char *argv[])
{
  global_data gd;
  urcu_init(2);

  std::thread th([&gd](){ worker(gd); });
  std::thread th2([&gd](){ worker(gd); });
  std::this_thread::sleep_for(std::chrono::seconds(1));
  gd.condvar.notify_all();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  std::cout << "hello\n";
  gd.stop = true;

  gd.skiplist.display();

  th.join();
  th2.join();

  return 0;
}
