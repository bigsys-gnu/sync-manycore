#pragma once
#ifndef UTILITY_H               // benchmark utility
#define UTILITY_H

#include <iostream>
#include <cstddef>
#include <random>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <string_view>
#include "skip_list.hh"
#include "tclap/CmdLine.h"

enum class benchmark_type
  {
    RCU,
    MV_RLU,
  };

struct statistics
{
  size_t add{0};
  size_t remove{0};
  size_t search{0};

  void print() const
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

  // default is add : remove : read = 1 : 1 : 1
  std::discrete_distribution<unsigned int> operation_dist{1, 1, 1};

  void set_operation_ratio(float read_ratio)
  {
    assert(read_ratio < 1);
    float write_ratio = 1 - read_ratio;
    operation_dist = std::discrete_distribution<unsigned int>{write_ratio / 2,
                                                              write_ratio / 2,
                                                              read_ratio};
  }

};

struct options
{
  TCLAP::CmdLine cmd;
  TCLAP::ValueArg<unsigned int> thread_num{"t", "thread_num",
                                           "the number of workers", false, 1u, ""};
  TCLAP::ValueArg<unsigned int> duration{"d", "benchmark_time",
                                         "benchmark duration in seconds", false, 10u,
                                         "seconds"};
  TCLAP::ValueArg<int> value_range{"r", "value_range",
                                   "skiplist key range from 0", false, 100000, ""};
  TCLAP::ValueArg<float> rw_ratio{"o", "rw_ratio", "skiplist read operation ratio",
                                  false, 0.8f, "float"};

  options(std::string_view welcome_msg, int argc, char *argv[]):
    cmd(welcome_msg.data())
  {
    cmd.add(thread_num);
    cmd.add(duration);
    cmd.add(value_range);
    cmd.add(rw_ratio);
    cmd.parse(argc, argv);
  }

  void
  init_global_data(global_data& gd)
  {
    gd.key_max = value_range.getValue();
    gd.thread_num = thread_num.getValue();
    gd.set_operation_ratio(rw_ratio.getValue());
  }

  void
  print_bench_stat()
  {
    std::cout << cmd.getMessage() << std::endl
              << "Thread Number:\t" << thread_num.getValue() << std::endl
              << "Benchmark Time:\t" << duration.getValue() << std::endl
              << "Key Range:\t 1 ~ " << value_range.getValue() << std::endl
              << "Read Ratio:\t" << rw_ratio.getValue() << std::endl;
  }
};

inline void gather_stat(global_data& gd, const statistics& local)
{
  std::unique_lock<std::mutex> sl(gd.stat_lock);
  auto& global_stat = gd.stat;
  global_stat.add += local.add;
  global_stat.remove += local.remove;
  global_stat.search += local.search;
}

#endif /* UTILITY_H */
