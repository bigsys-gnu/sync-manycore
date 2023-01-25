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
      workers.emplace_back(std::thread([&gd]()
      {
        worker(gd);
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
