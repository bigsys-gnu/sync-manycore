/**
	Program to perform benchmark testing on the skip list.
*/

#include <iostream>
#include <thread>
#include <random>
#include <condition_variable>
#include <vector>
#include "skip_list.hh"
#include "mvrlu_api.hh"
#include "utility.hh"
#include "zipf.hh"

template <typename RandomDist>
void worker(global_data& gd, RandomDist key_dist)
{
  std::mt19937 engine{std::random_device{}()};
  std::discrete_distribution<unsigned int> dist = gd.operation_dist;
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
          if (gd.skiplist.add(key))
            local_stat.add++;
          break;
        case 1:
          if (gd.skiplist.remove(key))
            local_stat.remove++;
          break;
        case 2:
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
  options ops("MV-RLU Skip List benchmark", argc, argv);

  ops.init_global_data(gd);
  ops.print_bench_stat();

  mvrlu_api::system mvrlu_system;
  std::vector<std::thread> workers;

  for (size_t i = 0; i < gd.thread_num; i++)
    {
      workers.emplace_back(mvrlu_api::create_thread([&gd]()
      {
        switch (gd.dist_type) {
        case workload_dist::UNIFORM:
          worker(gd, std::uniform_int_distribution<int>(1, gd.key_max));
          break;
        case workload_dist::ZIPF:
          worker(gd, custom_random::zipf_distribution<int, double>(gd.key_max));
          break;
        case workload_dist::NORMAL:
          break;
        default:
          break;
        }
      }));
    }

  std::this_thread::sleep_for(std::chrono::seconds(1));
  gd.condvar.notify_all();
  std::this_thread::sleep_for(std::chrono::seconds(ops.duration.getValue()));
  std::cout << "now let's stop them\n";
  gd.stop = true;

  for (size_t i = 0; i < workers.size(); i++)
    {
      workers[i].join();
    }

  gd.stat.print();
  mvrlu_system.~system();
  mvrlu_api::print_stats();

  return 0;
}
