/*
 *  random.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef RANDOM_H
#define RANDOM_H

// C++ includes:
#include <memory>
#include <random>
#include <utility>

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

namespace nest
{

using binomial_param_type = std::binomial_distribution<>::param_type;
using gamma_param_type = std::gamma_distribution<>::param_type;
using poisson_param_type = std::poisson_distribution<>::param_type;

struct BaseRNG; // forward declaration
using RngPtr = std::shared_ptr< BaseRNG >;
using rng_result_type = unsigned long;

struct BaseRNG
{
  virtual rng_result_type operator()() = 0;
  virtual RngPtr clone( unsigned long seed ) = 0;
  virtual double drand() = 0;
  virtual unsigned long ulrand( unsigned long N ) = 0;
  virtual rng_result_type min() = 0;
  virtual rng_result_type max() = 0;
  virtual void seed( unsigned long seed ) = 0;
};

template < typename RNG_TYPE_ >
struct RNG : public BaseRNG
{
  using result_type = rng_result_type;

  RNG(){};

  RNG( result_type seed )
    : rng_( seed )
  {
  }

  RNG( RNG_TYPE_ rng )
    : rng_( rng )
    , uniform_double_dist_0_1_( 0.0, 1.0 )
  {
  }

  inline result_type operator()() override
  {
    return rng_();
  }

  inline RngPtr
  clone( result_type seed ) override
  {
    return std::make_shared< RNG< RNG_TYPE_ > >( seed );
  }

  inline double
  drand() override
  {
    return uniform_double_dist_0_1_( rng_ );
  }

  inline unsigned long
  ulrand( unsigned long N )
  {
    std::uniform_int_distribution< unsigned long >::param_type param( 0, N - 1 );
    return uniform_ulong_dist_( rng_, param );
  }

  inline result_type
  min() override
  {
    return rng_.min();
  }

  inline result_type
  max() override
  {
    return rng_.max();
  }

  inline void
  seed( result_type seed ) override
  {
    return rng_.seed( seed );
  }

private:
  RNG_TYPE_ rng_;
  std::uniform_int_distribution< unsigned long > uniform_ulong_dist_;
  std::uniform_real_distribution<> uniform_double_dist_0_1_;
};

template < typename RNG_TYPE_ >
RngPtr
make_rng()
{
  return make_rng< RNG_TYPE_ >( 0 );
}

template < typename RNG_TYPE_ >
RngPtr
make_rng( unsigned long seed )
{
  return std::make_shared< RNG< RNG_TYPE_ > >( seed );
}


} // namespace nest

#endif /* #ifndef RNG_H */
