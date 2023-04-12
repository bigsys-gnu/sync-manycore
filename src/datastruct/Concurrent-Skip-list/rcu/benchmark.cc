/**
	Program to perform benchmark testing on the skip list.
*/

#include <iostream>
#include <thread>
#include <random>
#include <condition_variable>
#include <vector>
#include "rcu_api.hh"
#include "skip_list.hh"
#include "utility.hh"
#include "statistics.hh"


template <typename RandomDist>
void worker(global_data& gd, RandomDist key_dist)
{
  std::mt19937 engine{std::random_device{}()};
  std::discrete_distribution<unsigned int> dist = gd.operation_dist;
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
          if (gd.skiplist.add(key))
            local_stat.add++;
          break;
        case 1:
          if (gd.skiplist.remove(key))
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
  rcu_stat::gather_stat();
}

int main(int argc, char *argv[])
{
  global_data gd;
  std::vector<std::thread> workers;
  options ops("RCU Skip List benchmark", argc, argv);

  ops.init_global_data(gd);
  ops.print_bench_stat();

  rcu_api::init(gd.thread_num);

  for (int i = 0; i < gd.thread_num; i++)
    {
      workers.emplace_back(std::thread([&gd, &ops]()
      {
        switch (gd.dist_type) {
        case workload_dist::UNIFORM:
          worker(gd, std::uniform_int_distribution<int>(1, gd.key_max));
          break;
        case workload_dist::ZIPF:
          worker(gd, custom_random::
                 zipf_distribution<int, double>(gd.key_max,
                                                ops.zipf_s.getValue()));
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

  std::for_each(workers.begin(), workers.end(), [](auto&& w)
  {
    w.join();
  });

  gd.stat.print();
  rcu_stat::print_stat();

  return 0;
}
