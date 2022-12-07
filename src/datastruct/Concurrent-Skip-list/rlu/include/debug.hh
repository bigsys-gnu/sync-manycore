#pragma once
#include <iostream>
#include <string_view>
#include <type_traits>
#include "node.hh"
#include "skip_list.hh"

#ifndef MVRLU_DEBUG_H
#define MVRLU_DEBUG_H

namespace mvrlu_debug
{
  using namespace std::string_view_literals;

  #ifdef DEBUG
  constexpr const bool enable_debug = true;
  #else
  constexpr const bool enable_debug = false;
  #endif

  template <typename ...Args, typename Cond>
  void assert(std::string_view msg, Cond condition, Args... args)
  {
    if constexpr (enable_debug)
      {
        if (condition(args...))
          {
            std::cerr << msg << std::endl;
            std::terminate();
          }
      }
  }

  template <typename Arg, typename Cond>
  void assert(std::string_view msg, Cond condition, Arg arg)
  {
    if constexpr (enable_debug)
      {
        if (condition(arg))
          {
            std::cerr << msg << std::endl;
            std::terminate();
          }
      }
  }

  // assume that node has no zero value
  inline bool check_zero_value(const node_ptr ptr)
  {
    return ptr->get_key() == 0;
  }
  inline auto has_zero_value = [](const node_ptr ptr)
  {
    assert("Node has zero value!!!"sv, check_zero_value, ptr);
  };

}

#endif /* DEBUG_H */
