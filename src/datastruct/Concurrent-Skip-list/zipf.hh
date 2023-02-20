//=  C Version Authors...                                                   =
//=-------------------------------------------------------------------------=
//=  Author: Kenneth J. Christensen                                         =
//=          University of South Florida                                    =
//=          WWW: http://www.csee.usf.edu/~christen                         =
//=          Email: christen@csee.usf.edu                                   =
//=-------------------------------------------------------------------------=
//=  History: KJC (11/16/03) - Genesis (from genexp.c)                      =
//===========================================================================
//=  C++ Version Author...                                                  =
//=  Author: Chang-Hui Kim                                                  =
//=          Gyeongsang National University                                 =
//=          Email: kch9001@gmail.com                                       =
#pragma once
#ifndef ZIPF_H
#define ZIPF_H
#include <cmath>
#include <random>
#include <cassert>
#include <iostream>

namespace custom_random
{
  class zipf_generator
  {
    inline static double c_;
    inline static double alpha_;
    inline static int max_value_;
    std::uniform_real_distribution<double> dist_{0, 1};

  public:
    zipf_generator() = default;

    static void init_constants(double alpha, std::size_t max)
    {
      alpha_ = alpha;
      max_value_ = max;
      for (std::size_t i = 1; i <= max; i++)
        {
          c_ += (1.0 / std::pow(double(i), alpha));
        }
      c_ = 1.0 / c_;
    }

    static double get_C()
    {
      return zipf_generator::c_;
    }

    static double get_alpha()
    {
      return zipf_generator::alpha_;
    }

    static std::size_t get_max()
    {
      return zipf_generator::max_value_;
    }

    template <typename RandomEngine>
    int operator () (RandomEngine& rng) // generate zipf random value
    {
      double z = 0;
      do
        {
          z = dist_(rng);
        }
      while(z == 0 || z == 1);

      double sum_prob = 0;
      std::size_t zipf_value = 1;
      for (std::size_t i = 1; i <= get_max(); i++)
        {
          sum_prob = sum_prob + get_C() / std::pow(double(i), get_alpha());
          if (sum_prob >= z)
            {
              zipf_value = i;
              break;
            }
        }

      assert(zipf_value >= 1 && zipf_value <= get_max());
      return zipf_value;
    }

  };

}    

#endif /* ZIPF_H */
