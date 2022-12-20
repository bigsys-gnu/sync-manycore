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
#include <cassert>
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
  unsigned int thread_num{1};
  int key_max{1000000};
  std::mutex cond_lock;
  std::condition_variable condvar;
  std::mutex stat_lock;
  bool stop = false;
  SkipList skiplist;
  statistics stat;
  std::discrete_distribution<unsigned int> operation_dist{1, 1, 1}; // default is add : remove : read = 1 : 1 : 1

  void set_operation_ratio(float read_ratio)
  {
    assert(read_ratio < 1);
    float write_ratio = 1 - read_ratio;
    operation_dist = std::discrete_distribution<unsigned int>{write_ratio / 2, write_ratio / 2, read_ratio};
  }

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
  std::discrete_distribution<unsigned int> dist = gd.operation_dist;
  std::uniform_int_distribution<int> key_dist(1, gd.key_max);
  rcu_api::regist();

  std::unique_lock<std::mutex> lk(gd.cond_lock); // hold lock
  gd.condvar.wait(lk);
  lk.unlock();

  statistics local_stat;
  while (!gd.stop)
    {
      auto op = dist(engine);
      auto key = key_dist(engine);

      switch (op)
        {
        case 0:
          gd.skiplist.add(key);
          local_stat.add++;
          break;
        case 1:
          gd.skiplist.remove(key);
          local_stat.remove++;
          break;
        case 2:
          rcu_api::reader_scope read_session;
          gd.skiplist.search(key);
          local_stat.search++;
          break;
        }
    }
  gather_stat(gd, local_stat);
}

int main(int argc, char *argv[])
{
  global_data gd;
  std::vector<std::thread> workers;
  TCLAP::CmdLine cmd("rcu benchmark options");
  TCLAP::ValueArg<unsigned int> thread_num("t", "thread_num",
                                           "the number of workers", false, 1u, "");
  TCLAP::ValueArg<unsigned int> duration("d", "benchmark_time",
                                         "benchmark duration in seconds", false, 10u, "");
  TCLAP::ValueArg<int> value_range("r", "value_range",
                                   "skiplist key range from 0", false, 100000, "");
  TCLAP::ValueArg<float> rw_ratio("o", "rw_ratio", "skiplist read operation ratio",
                                  false, 0.8f, "float");

  cmd.add(thread_num);
  cmd.add(duration);
  cmd.add(value_range);
  cmd.add(rw_ratio);
  cmd.parse(argc, argv);

  gd.key_max = value_range.getValue();
  gd.thread_num = thread_num.getValue();
  gd.set_operation_ratio(rw_ratio.getValue());

  std::cout << "Thread Number:\t" << thread_num.getValue() << std::endl
            << "Benchmark Time:\t" << duration.getValue() << std::endl
            << "Key Range:\t 1 ~ " << value_range.getValue() << std::endl
            << "Read Ratio:\t" << rw_ratio.getValue() << std::endl;

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
