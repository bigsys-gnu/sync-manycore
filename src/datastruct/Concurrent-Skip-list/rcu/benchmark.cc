/**
	Program to perform benchmark testing on the skip list.
*/

#include <iostream>
#include <thread>
#include <random>
#include <condition_variable>
#include <string>
#include <vector>
#include <algorithm>
#include "rcu_api.hh"
#include "skip_list.hh"

struct statistics
{
  size_t add{0};
  size_t remove{0};
  size_t search{0};

  void print()
  {
    std::cout << "Add:\t" << add << std::endl
              << "Remove:\t" << remove << std::endl
              << "Search:\t" << search << std::endl;
  }
};

struct global_data
{
  unsigned int thread_num{8};
  std::default_random_engine engine;
  std::uniform_int_distribution<unsigned int> dist = std::uniform_int_distribution<unsigned int>(1, 3);
  std::uniform_int_distribution<int> key_dist = std::uniform_int_distribution<int>(1, 1000000);
  std::mutex cond_lock;
  std::condition_variable condvar;
  std::mutex stat_lock;
  bool stop = false;
  SkipList skiplist;
  statistics stat;
};

void gather_stat(global_data& gd, const statistics& local)
{
  std::unique_lock<std::mutex> sl(gd.stat_lock);
  auto& global_stat = gd.stat;
  global_stat.add += local.add;
  global_stat.remove += local.remove;
  global_stat.search += local.search;
}

void worker(global_data& gd)
{
  rcu_api::regist();
  std::cout << "registered\n";
  std::unique_lock<std::mutex> lk(gd.cond_lock); // hold lock
  std::cout << "sleep for cond\n";
  gd.condvar.wait(lk);
  lk.unlock();
  std::cout << "wait is done\n";

  statistics local_stat;
  while (!gd.stop)
    {
      auto op = gd.dist(gd.engine);
      auto data = "hello RCU";
      auto key = gd.key_dist(gd.engine);

      switch (op)
        {
        case 1:
          gd.skiplist.add(key, data);
          local_stat.add++;
          break;
        case 2:
          gd.skiplist.remove(key);
          local_stat.remove++;
          break;
        case 3:
          rcu_api::reader_scope read_session;
          gd.skiplist.search(key);
          local_stat.search++;
          break;
        }
    }

  std::cout << "byebye\n";
  gather_stat(gd, local_stat);
}


int main(int argc, char *argv[])
{
  global_data gd;
  std::vector<std::thread> workers;
  rcu_api::init(gd.thread_num);

  for (int i = 0; i < gd.thread_num; i++)
    {
      workers.emplace_back(std::thread([&gd]()
      {
        worker(gd);
      }));
    }

  std::this_thread::sleep_for(std::chrono::seconds(1));
  gd.condvar.notify_all();
  std::this_thread::sleep_for(std::chrono::seconds(30));
  std::cout << "now let's stop them\n";
  gd.stop = true;

  std::for_each(workers.begin(), workers.end(), [](auto&& w)
  {
    w.join();
  });

  gd.stat.print();

  return 0;
}
