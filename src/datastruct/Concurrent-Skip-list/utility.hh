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
#include <string>
#include <vector>
#include <tclap/CmdLine.h>
#include "skip_list.hh"
#include "zipf.hh"

enum class benchmark_type
  {
    RCU,
    MV_RLU,
  };

enum class workload_dist
  {
    UNIFORM,
    ZIPF,
    NORMAL,
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
  workload_dist dist_type = workload_dist::UNIFORM;

  // default is add : remove : read = 1 : 1 : 1
  std::discrete_distribution<unsigned int> operation_dist{{1, 1, 1}};

  void set_operation_ratio(float read_ratio)
  {
    assert(read_ratio < 1);
    float write_ratio = 1 - read_ratio;
    operation_dist = std::discrete_distribution<unsigned int>{ {write_ratio / 2,
                                                                  write_ratio / 2,
                                                                  read_ratio} };
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

  TCLAP::ValuesConstraint<std::string> workload_types{std::vector<std::string>{"uniform", "zipf", "normal"}};
  TCLAP::ValueArg<std::string> workload_dist{"w", "workload_dist",
                                             "workload random distribution", false,
                                             "uniform", &workload_types};
  TCLAP::ValueArg<float> zipf_s{"s", "zipf_s",
                                "zipfian distribution exponent value", false,
                                1.0, "float"};

  options(std::string_view welcome_msg, int argc, char *argv[]):
    cmd(welcome_msg.data())
  {
    cmd.add(thread_num);
    cmd.add(duration);
    cmd.add(value_range);
    cmd.add(rw_ratio);
    cmd.add(workload_dist);
    cmd.add(zipf_s);
    cmd.parse(argc, argv);
  }

  void
  init_global_data(global_data& gd)
  {
    gd.key_max = value_range.getValue();
    gd.thread_num = thread_num.getValue();
    gd.set_operation_ratio(rw_ratio.getValue());
    if (workload_dist.getValue() == "uniform")
      {
        gd.dist_type = workload_dist::UNIFORM;
      }
    else if (workload_dist.getValue() == "zipf")
      {
        gd.dist_type = workload_dist::ZIPF;
      }
    else
      {
        gd.dist_type = workload_dist::NORMAL;
      }
  }

  void
  print_bench_stat()
  {
    std::cout << cmd.getMessage() << '\n'
              << "Thread Number:\t" << thread_num.getValue() << '\n'
              << "Benchmark Time:\t" << duration.getValue() << '\n'
              << "Key Range:\t 1 ~ " << value_range.getValue() << '\n'
              << "Read Ratio:\t" << rw_ratio.getValue() << '\n'
              << "Random Distribution:\t" << workload_dist.getValue() << '\n';
    if (workload_dist.getValue() == "zipf")
      {
        std::cout << "Zipf Exponent:\t" << zipf_s.getValue() << '\n';
      }
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
