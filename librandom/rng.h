/*
 *  rng.h
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

#ifndef RNG_H
#define RNG_H

// Includes from librandom:
#include "librandom.h"

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

// C++ includes
#include <random>


namespace nest {

namespace random {

template<typename RNG_TYPE_ >
class RNG : public BaseRNG
{
  RNG() {};

  RNG( RNG_TYPE_ rng )
    : rng_( rng )
    , uniform_dist_0_1_(0.0, 1.0)
  {
  }

public:
  inline int operator()()
  {
    return rng_();
  }

  inline BaseRNG* clone( long seed )
  {
    return new RNG { RNG_TYPE_ { seed } };
  }

  inline double drand()
  {
    return uniform_dist_0_1_(rng_);
  }

  inline double min()
  {
    return rng_.min();
  }

  inline double max()
  {
    return rng_.max();
  }

private:
  RNG_TYPE_ rng_;
  std::uniform_real_distribution<> uniform_dist_0_1_;
};

//class Philox : public BaseRNG
//{
//};
//
//
//class Threefry : public BaseRNG
//{
//};

} // namespace random

} // namespace nest

#endif /* #ifndef RNG_H */
