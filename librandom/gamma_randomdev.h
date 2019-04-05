/*
 *  gamma_randomdev.h
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

#ifndef GAMMA_RANDOMDEV_H
#define GAMMA_RANDOMDEV_H

// C++ includes:
#include <cmath>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"

/************************************************************/
/* Class GammaRNG                                           */
/*                                                          */
/* Generates an RNG which returns gamma(x; a) distributed   */
/* random numbers out of an RNG which returns uniformly     */
/* distributed random numbers:                              */
/*                                                          */
/*       gamma(x; a) = x^(a-1) * exp(-x) / Gamma(a)         */
/*                                                          */
/* gamma(x; a, b) distributed random number are obtained by */
/* scaling: if X ~ gamma(x; a), then b*X ~ gamma(x; a, b).  */
/*                                                          */
/* Arguments:                                               */
/*  - pointer to an RNG                                     */
/*  - parameters a (optional, default = 1)                  */
/*                                                          */
/* Algorithm:                                               */
/*  a < 1 : Johnk's algorithm [1, p. 418]                   */
/*  a = 1 : direct transformation (exponential distribution)*/
/*  a > 1 : Best's algorithm [1, p. 410]                    */
/*                                                          */
/* Verification:                                            */
/*  Kolmogorov-Smirnov test at p=0.01 passed for sample     */
/*  size 10,000 and orders 0.01 - 25 (MATLAB Stats Toolbox) */
/*                                                          */
/* Author:                                                  */
/*  Hans Ekkehard Plesser                                   */
/*                                                          */
/* References:                                              */
/*  [0] always reserved for Stroustrup                      */
/*  [1] L. Devroye, "Non-Uniform Random Variate Generation",*/
/*      Springer, 1986                                      */
/*                                                          */
/************************************************************/

namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::gamma - gamma random deviate generator

Description:
   Generates gamma-distributed random numbers.

   gamma(x; order, b) = x^(order-1) * exp(-x/b) / b^order Gamma(order) , x >= 0

Parameters:
   order - order of the gamma distribution (default: 1)
   b - scale parameter (default: 1)

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Class GammaRandomDev Create gamma distributed random numbers
 *
 * @ingroup RandomDeviateGenerators
 */

class GammaRandomDev : public RandomDev
{

public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  GammaRandomDev( RngPtr, double a_in = 1.0 ); //!< create with fixed RNG
  GammaRandomDev(
    double a_in = 1.0 ); //!< create w/o fixed RNG for threaded simulations

  void set_order( double ); //!< set order
  void set_scale( double ); //!< set scale parameter

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

  using RandomDev::operator();
  double operator()( RngPtr ) const; //!< draw number, threaded
  double operator()( RngPtr,
    double ); //!< draw number, threaded, explicit order

private:
  double unscaled_gamma(
    RngPtr r ) const; //! worker function creating Gamma(x; order, 1) number

  double a;  // gamma density order
  double b_; // gamma scale parameter

  double bb; // parameters b, c of Best's algorithm
  double bc;
  double ju; // exponents of U, V of Johnk's algorithm
  double jv;
};

inline double GammaRandomDev::operator()( RngPtr rthrd, double a )
{
  set_order( a );
  return ( *this )( rthrd );
}

inline double GammaRandomDev::operator()( RngPtr r ) const
{
  return b_ * unscaled_gamma( r );
}

inline void
GammaRandomDev::set_order( double a_in = 1.0 )
{
  assert( a_in > 0 );

  a = a_in;

  bb = a - 1.0;
  bc = 3.0 * ( a - 0.25 );

  ju = 1.0 / a;
  jv = a != 1 ? 1.0 / ( 1 - a ) : 0;
}
}

#endif
