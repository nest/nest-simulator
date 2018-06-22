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

class MT19937 : public BaseRNG
{
  MT19937() {};

  MT19937( std::mt19937_64 rng )
    : rng_( rng )
  {  
  }

public:
  inline int operator()()
  {
    return rng_();
  }
    
  inline BaseRNG* clone( long seed )
  {
    return new MT19937 { std::mt19937_64 { seed } };
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
  std::mt19937_64 rng_;
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
