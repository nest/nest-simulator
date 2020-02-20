/*
 *  normal_randomdev.h
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

#ifndef NORMAL_RANDOMDEV_H
#define NORMAL_RANDOMDEV_H

// C++ includes:
#include <cmath>

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"


namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::normal - normal random deviate generator
Description: Generates normally distributed random numbers.

  p(x) = 1 / (sigma * \sqrt{2 pi}) * exp (-(x-mu)^2 / 2 sigma^2)

Parameters:
 mu  - mean (default: 0.0)
 sigma - standard deviation (default: 1.0)

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Create normal (Gaussian) random numbers with uniform variance.
 * The numbers are generated using the polar method.
 *
 * @note We cannot keep the second deviate until the next call,
 *       since the generator may be called by a different thread
 *       with its own RNG.7
 *
 * @ingroup RandomDeviateGenerators
 */

class NormalRandomDev : public RandomDev
{

public:
  // accept only shared_ptrs for initialization,
  // otherwise creation of a shared_ptr would
  // occur as side effect---might be unhealthy
  NormalRandomDev( RngPtr );
  NormalRandomDev(); // threaded

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
