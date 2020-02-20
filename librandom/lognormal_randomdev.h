/*
 *  lognormal_randomdev.h
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

#ifndef LOGNORMAL_RANDOMDEV_H
#define LOGNORMAL_RANDOMDEV_H

// C++ includes:
#include <cmath>

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"


namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::lognormal - lognormal random deviate generator

Description: Generates lognormally distributed random numbers.

  p(x) = 1 / (x * sigma * \sqrt{2 pi}) * exp (-(ln(x)-mu)^2 / 2 sigma^2)

Parameters:
 mu  - mean of the underlying normal distribution (default: 0.0)
 sigma - standard deviation of the underlying normal distribution (default: 1.0)

Remarks:
Mean and variance of the lognormal numbers are given by

  E[X] = exp(mu + sigma^2/2)
  Var[X] = (exp(sigma^2) - 1) * E[X]^2

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Create lognormal random numbers with uniform variance.
 * @ingroup RandomDeviateGenerators
 */

class LognormalRandomDev : public RandomDev
{

public:
  // accept only shared_ptrs for initialization,
  // otherwise creation of a shared_ptr would
  // occur as side effect---might be unhealthy
  LognormalRandomDev( RngPtr );
  LognormalRandomDev(); // threaded

  using RandomDev::operator();
  double operator()( RngPtr ) const; // threaded

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  double mu_;
  double sigma_;
};
}

#endif
