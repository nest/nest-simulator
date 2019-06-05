/*
 *  poisson_randomdev.h
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

#ifndef POISSON_RANDOMDEV_H
#define POISSON_RANDOMDEV_H

// C++ includes:
#include <cmath>
#include <vector>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"

/************************************************************/
/* Class PoissonRNG                                         */
/*                                                          */
/* Generates an RNG which returns Poisson(x; lambda)        */
/* distributed random numbers out of an RNG which returns   */
/* uniformly distributed random numbers:                    */
/*                                                          */
/*    p(n) = (lambda^n / n!) exp(-lambda)  , n = 0, 1, ...  */
/*                                                          */
/* Arguments:                                               */
/*  - pointer to an RNG                                     */
/*  - parameters lambda (optional, default = 1)             */
/*                                                          */
/* Algorithm: Ahrens & Dieter [1]                           */
/*  - table lookup for lambda < 10                          */
/*  - involved rejection scheme based on normal random dev  */
/*    otherwise                                             */
/*  - answer to [2], Sec 3.4.1, exercise 8                  */
/*  - changing lambda involves some costly computations     */
/*                                                          */
/* Verification:                                            */
/*  - 60 different lambda, 0.01 .. 100                      */
/*  - 10.000.000 numbers generated per lambda               */
/*  - mt19937 as uniform rng source                         */
/*  - chi^2 test on batches of 10000 numbers                */
/*  - Kolmogorov-Smirnov test on chi^2 test scores          */
/*  - lambda == 0.01 critical, most likely due problems     */
/*    with test (just two bins in chi^2 test)               */
/*  - lambda == 20 failed KS test once, passed on second    */
/*    set of 10^7 numbers generated with different seed     */
/*                                                          */
/* References:                                              */
/* [1] J H Ahrens, U Dieter, ACM TOMS 8:163-179(1982)       */
/* [2] D E Knuth, The Art of Computer Programming, vol 2.   */
/*                                                          */
/* Author:                                                  */
/*  Hans Ekkehard Plesser                                   */
/*                                                          */
/* History:                                                 */
/*  3.0 HEP, 2003-05-23 Ahrens & Dieter algorithm           */
/*  2.0 HEP, 2002-07-09 (for librandom 2.0)                 */
/*                                                          */
/* To do:                                                   */
/*  - some of the numerics has only floating point accuracy */
/*    should be improved to double.                         */

/************************************************************/

namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::poisson - poisson random deviate generator

Description:
   Generates poisson distributed random numbers.

   p(n) = (lambda^n / n!) exp(-lambda)  , n = 0, 1, ...

Parameters:
   lambda - distribution parameter, lambda

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Class PoissonRandomDev Create Poisson distributed random numbers
 *
 * @ingroup RandomDeviateGenerators
 */

class PoissonRandomDev : public RandomDev
{
  RngPtr r; // pointer to underlying uniform RNG

public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  PoissonRandomDev( RngPtr, double lambda = 0.0 );
  PoissonRandomDev( double lambda = 0.0 ); // for threaded environments

  void set_lambda( double );

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

  /**
   * Import sets of overloaded virtual functions.
   * We need to explicitly include sets of overloaded
   * virtual functions into the current scope.
   * According to the SUN C++ FAQ, this is the correct
   * way of doing things, although all other compilers
   * happily live without.
   */
  using RandomDev::operator();
  using RandomDev::ldev;

  long ldev( RngPtr ) const; //!< draw integer, threaded
  bool
  has_ldev() const
  {
    return true;
  }

  double operator()( RngPtr ) const; //!< return as double, threaded

private:
  void init_(); //!< re-compute internal parameters

  double mu_; //!< Poisson parameter, aka lambda

  // quantities for case A, steps N, I, S
  double s_;        //!< sqrt(mu_)
  double d_;        //!< 6 mu_^2
  unsigned long L_; //!< floor(mu-1.1484)

  // quantities for case A, step H
  double c_; //!< 0.1069 / mu_

  // quantities for case A, function F
  double om_;
  double c0_;
  double c1_;
  double c2_;
  double c3_;

  static const unsigned n_tab_; //!< tabulate P_0 ... P_{n_tab_-1}
  std::vector< double > P_;     //!< PoissonCDF

  static const unsigned fact_[]; //!< array of factorials 0! .. 10!

  static const double a_[];   //!< array of a_i coeffs
  static const unsigned n_a_; //!< length of array

  //! Procedure F from Ahrens & Dieter
  void proc_f_( const unsigned k,
    double& px,
    double& py,
    double& fx,
    double& fy ) const;
};
}

inline double librandom::PoissonRandomDev::operator()( RngPtr rthrd ) const
{
  return static_cast< double >( ldev( rthrd ) );
}

#endif
