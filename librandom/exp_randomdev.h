/*
 *  exp_randomdev.h
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

#ifndef EXP_RANDOMDEV_H
#define EXP_RANDOMDEV_H

// C++ includes:
#include <cmath>

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"

namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::exponential - exponential random deviate generator

Description: Generates exponentially distributed random numbers.
Negative values of lambda are allowed and generate a distribution
of negative numbers.

For lambda > 0:
  p(x) = lambda exp(-lambda*x), for x >= 0
  p(x) = 0, for x < 0

For lambda < 0:
  p(x) = 0, for x > 0
  p(x) = |lambda| exp ( -|lambda| |x| ), for x <= 0

Parameters:
 lambda - rate parameter (default: 1.0)

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Class ExpRandomDev Create exponential random numbers
 *
 * @ingroup RandomDeviateGenerators
 */

class ExpRandomDev : public RandomDev
{

public:
  // accept only shared_ptrs for initialization,
  // otherwise creation of a shared_ptr would
  // occur as side effect---might be unhealthy
  ExpRandomDev( RngPtr r_in )
    : RandomDev( r_in )
    , lambda_( 1.0 )
  {
  }
  ExpRandomDev()
    : RandomDev()
    , lambda_( 1.0 )
  {
  } // threaded

  using RandomDev::operator();
  double operator()( RngPtr rthrd ) const; // threaded

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  double lambda_; //!< rate parameter
};

inline double ExpRandomDev::operator()( RngPtr rthrd ) const
{
  return -std::log( rthrd->drandpos() ) / lambda_;
}
}

#endif
