/*
 *  binomial_randomdev.h
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

// Generated includes:
#include "config.h"

#ifndef BINOMIAL_RANDOMDEV_H
#define BINOMIAL_RANDOMDEV_H

// C++ includes:
#include <cmath>
#include <vector>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from librandom:
#include "exp_randomdev.h"
#include "poisson_randomdev.h"
#include "randomdev.h"
#include "randomgen.h"

// Includes from sli:
#include "dictdatum.h"


/** @BeginDocumentation
Name: rdevdict::binomial - binomial random deviate generator

Description:
   Generates binomially distributed random numbers.

   p(k) = (n! / k!(n-k)!) p^k (1-p)^(n-k)  , 0<=k<=n, n>0

Parameters:
   p - probability of success in a single trial (double)
   n - number of trials (positive integer)

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Hans Ekkehard Plesser, Moritz Deger
*/


namespace librandom
{

/**
 Class BinomialRNG

 Generates an RNG which returns Binomial(k;p;n)
 distributed random numbers out of an RNG which returns
 binomially distributed random numbers:

    p(k) = (n! / k!(n-k)!) p^k (1-p)^(n-k)  , 0<=k<=n, n<0

 Arguments:
  - pointer to an RNG
  - parameter p (optional, default = 0.5)
  - parameter n (optional, default = 1)

 @see Fishman, Sampling From the Binomial Distribution on a Computer, Journal of
 the American Statistical Association, Vol. 74, No. 366 (Jun., 1979),
 pp. 418-423
 @ingroup RandomDeviateGenerators
*/

/* ----------------------------------------------------------------
 * Draw a binomial random number using the BP algoritm
 * Sampling From the Binomial Distribution on a Computer
 * Author(s): George S. Fishman
 * Source: Journal of the American Statistical Association, Vol. 74, No. 366
 * (Jun., 1979), pp.
 * 418-423
 * Published by: American Statistical Association
 * Stable URL: http://www.jstor.org/stable/2286346 .
 * ---------------------------------------------------------------- */


class BinomialRandomDev : public RandomDev
{
public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  BinomialRandomDev( RngPtr, double p_s = 0.5, unsigned int n_s = 1 );
  BinomialRandomDev( double p_s = 0.5, unsigned int n_s = 1 );

  /**
   * set parameters for p and n
   * @parameters
   * p - success probability for single trial
   * n - number of trials
   */
  void set_p_n( double, unsigned int );
  void set_p( double );       //!<set p
  void set_n( unsigned int ); //!<set n

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

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;


private:
  PoissonRandomDev poisson_dev_; //!< source of Poisson random numbers
  ExpRandomDev exp_dev_;         //!< source of exponential random numbers
  double p_;                     //!<probability p of binomial distribution
  double phi_;
  long m_;
  unsigned int n_;          //!<parameter n in binomial distribution
  std::vector< double > f_; //!< precomputed table of f
  unsigned int n_tablemax_; //!< current maximal n with precomputed values

  void init_();                   //!< check and initialize internal parameters
  void PrecomputeTable( size_t ); //!< compute the internal lookup table
};

inline double BinomialRandomDev::operator()( RngPtr rthrd ) const
{
  return static_cast< double >( ldev( rthrd ) );
}
}

#endif
