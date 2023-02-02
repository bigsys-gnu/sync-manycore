#pragma once
#ifndef STATISTICS_H
#define STATISTICS_H

#include <cstddef>
#include <iostream>
#include <string_view>
#include <mutex>

// stat for rcu benchmark
namespace rcu_stat
{
  constexpr static auto STAT_ON =
#ifdef STAT
  true;
#else
  false;
#endif

  enum class stats_name
    {
      ABORT_COUNT,
    };

  constexpr inline std::string_view get_name(stats_name sn)
  {
    switch (sn)
      {
      case stats_name::ABORT_COUNT:
        return {"ABORT_COUNT"};
      }
  }

  template <stats_name S, bool ON = true>
  class counter
  {
  private:
    std::size_t abort_counter{0};
    constexpr static std::string_view stat_name{get_name(S)};

  public:
    void operator++(int)
    {
      abort_counter++;
    }

    counter operator + (counter o)
    {
      return { abort_counter + o.abort_counter };
    }

    counter operator += (counter o)
    {
      abort_counter += o.abort_counter;
      return *this;
    }

    friend std::ostream& operator << (std::ostream& o, counter c)
    {
      o << c.stat_name << ": " << c.abort_counter << '\n';
      return o;
    }

  };

  template <stats_name S>
  class counter<S, false>
  {
  public:
    void operator++(int) {}
    counter& operator + (counter o) { return *this; }
    counter& operator += (counter o) { return *this; }
    friend std::ostream& operator << (std::ostream& o, counter c) { return o; }
  };

  template <bool ON = true>
  struct stats
  {
    counter<stats_name::ABORT_COUNT, ON> abort_counter;
  };

  inline thread_local rcu_stat::stats<rcu_stat::STAT_ON> local_stat;
  inline rcu_stat::stats<rcu_stat::STAT_ON> global_stat;
  inline std::mutex g_stat_lock;

  template <bool ON = true>
  void count_abort_impl()
  {
    local_stat.abort_counter++;
  }

  template <>
  inline void count_abort_impl<false>() {}

  inline void count_abort()
  {
    count_abort_impl<STAT_ON>();
  }

  template <bool ON = true>
  void gather_stat_impl()
  {
    std::unique_lock<std::mutex> lock{g_stat_lock};
    global_stat.abort_counter += local_stat.abort_counter;
  }

  template <>
  inline void gather_stat_impl<false>() {}

  inline void gather_stat()
  {
    gather_stat_impl<STAT_ON>();
  }

  template <bool ON = true>
  void print_stat_impl()
  {
    std::cout << "RCU STATISTICS\n"
              << global_stat.abort_counter << '\n';
  }

  template <>
  inline void print_stat_impl<false>() {}

  inline void print_stat()
  {
    print_stat_impl<STAT_ON>();
  }
}

#endif /* STATISTICS_H */
