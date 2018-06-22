/*
 *  rdist.h
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

#ifndef RDIST_H
#define RDIST_H

// Includes from librandom:
#include "librandom.h"

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

// C++ includes
#include <random>


namespace nest {

namespace random {

class binomial : public BaseRDist {
public:
  binomial( Name name)
    : BaseRDist ( name )
  {
  }

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  inline BaseRDist* clone()
  {
    return new binomial { name_ };
  }

  inline double drand( BaseRNG& rng )
  {
    return dist_( rng );
  }

  inline ResultType get_result_type()
  {
    return ResultType::continuous;
  }

private:
  std::binomial_distribution<long> dist_;
};

//class exponential : public BaseRDist {};
//
//class gamma : public BaseRDist {};
//
//class lognormal : public BaseRDist {};
//
//class normal : public BaseRDist {};
//
//class poisson : public BaseRDist {};
//
//class uniform_int : public BaseRDist {};
//
//class uniform_real : public BaseRDist {};

} // namespace random

} // namespace nest

#endif /* #ifndef RNG_H */
