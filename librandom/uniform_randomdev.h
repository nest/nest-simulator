/*
 *  uniform_randomdev.h
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

#ifndef UNIFORM_RANDOMDEV_H
#define UNIFORM_RANDOMDEV_H

// C++ includes:
#include <cmath>

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"

/************************************************************/
/* Class UniformRNG                                         */
/*                                                          */
/* Generates an RNG which returns integer random numbers    */
/* uniformly distributed between two limits.                */
/*                                                          */
/* Arguments:                                               */
/*  - pointer to an RNG                                     */
/*                                                          */
/* Author:                                                  */
/*  Hans Ekkehard Plesser                                   */
/*                                                          */
/* History:                                                 */
/*  HEP, 2004-08-05                                         */
/*                                                          */
/************************************************************/

namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::uniform - uniform random deviate generator
Description: Generates uniformly distributed numbers in the interval
             [low, high).

Parameters:
  low  - lower interval boundary, included
  high - upper interval boudnary, excluded

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Class UniformRandomDev
 * Create uniformly distributed random numbers in [low, high).
 *
 * @ingroup RandomDeviateGenerators
 */

class UniformRandomDev : public RandomDev
{

public:
  // accept only shared_ptrs for initialization,
  // otherwise creation of a shared_ptr would
  // occur as side effect---might be unhealthy
  UniformRandomDev( RngPtr r_in );
  UniformRandomDev(); // threaded

  using RandomDev::operator();
  double operator()( RngPtr rthrd ) const; // threaded

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  double low_;   //!< lower bound, included
  double high_;  //!< upper bound, excluded
  double delta_; //!< interval width
};

inline double UniformRandomDev::operator()( RngPtr rthrd ) const
{
  return low_ + delta_ * rthrd->drand();
}
}
#endif
