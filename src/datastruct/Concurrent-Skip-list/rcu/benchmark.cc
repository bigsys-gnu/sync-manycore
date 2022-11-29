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
#include "tclap/CmdLine.h"

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
  int key_max{1000000};
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
  std::default_random_engine engine{std::random_device{}()};
  std::uniform_int_distribution<unsigned int> dist(1, 3);
  std::uniform_int_distribution<int> key_dist(1, gd.key_max);
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
      auto op = dist(engine);
      auto key = key_dist(engine);

      switch (op)
        {
        case 1:
          gd.skiplist.add(key);
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
  TCLAP::CmdLine cmd("rcu benchmark options");
  TCLAP::ValueArg<unsigned int> thread_num("t", "thread_num",
                                           "the number of workers", true, 1u, "");
  TCLAP::ValueArg<unsigned int> duration("d", "benchmark_time",
                                         "benchmark duration in seconds", true, 10u, "");
  TCLAP::ValueArg<int> value_range("r", "value_range",
                                   "skiplist key range from 0", true, 100000, "");

  cmd.add(thread_num);
  cmd.add(duration);
  cmd.add(value_range);
  cmd.parse(argc, argv);

  gd.key_max = value_range.getValue();
  gd.thread_num = thread_num.getValue();

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
  std::this_thread::sleep_for(std::chrono::seconds(duration.getValue()));
  std::cout << "now let's stop them\n";
  gd.stop = true;

  std::for_each(workers.begin(), workers.end(), [](auto&& w)
  {
    w.join();
  });

  gd.stat.print();

  return 0;
}
