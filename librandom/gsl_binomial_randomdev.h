/*
 *  gsl_binomial_randomdev.h
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

#ifndef GSL_BINOMIAL_RANDOMDEV_H
#define GSL_BINOMIAL_RANDOMDEV_H

// Includes from librandom:
#include "gslrandomgen.h"
#include "randomdev.h"
#include "randomgen.h"

// Includes from sli:
#include "dictdatum.h"

#ifdef HAVE_GSL

// External includes:
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>


/** @BeginDocumentation
Name: rdevdict::gsl_binomial - GSL binomial random deviate generator

Description:
This function returns a random integer from the binomial distribution,
the number of successes in n independent trials with probability
p. The probability distribution for binomial variates is,

   p(k) = (n! / k!(n-k)!) p^k (1-p)^(n-k)  , 0<=k<=n, n>0

Please note that the RNG used to initialize gsl_binomial has to be
from the GSL (prefixed gsl_ in rngdict)

Parameters:
   p - probability of success in a single trial (double)
   n - number of trials (positive integer)

SeeAlso: CreateRDV, RandomArray, rdevdict

Author: Jochen Martin Eppler
*/


namespace librandom
{

/**
 Class GSL_BinomialRandomDev

 Generates an RNG which returns Binomial(k;p;n)
 distributed random numbers out of an RNG which returns
 binomially distributed random numbers:

    p(k) = (n! / k!(n-k)!) p^k (1-p)^(n-k)  , 0<=k<=n, n<0

 Arguments:
  - pointer to an RNG
  - parameter p (optional, default = 0.5)
  - parameter n (optional, default = 1)

 @see
 http://www.gnu.org/software/gsl/manual/html_node/The-Binomial-Distribution.html
 @ingroup RandomDeviateGenerators
*/

class GSL_BinomialRandomDev : public RandomDev
{
public:
  // accept only shared_ptrs for initialization,
  // otherwise creation of a shared_ptr would
  // occur as side effect---might be unhealthy
  GSL_BinomialRandomDev( RngPtr, double p_s = 0.5, unsigned int n_s = 1 );
  GSL_BinomialRandomDev( double p_s = 0.5, unsigned int n_s = 1 );

  /**
   * set parameters for p and n
   * @parameters
   * p - success probability for single trial
   * n - number of trials
   */
  void set_p_n( double, size_t );
  void set_p( double ); //!<set p
  void set_n( size_t ); //!<set n

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

  long ldev();               //!< draw integer
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
  double p_;       //!<probability p of binomial distribution
  unsigned int n_; //!<parameter n in binomial distribution

  gsl_rng* rng_;
};

inline double GSL_BinomialRandomDev::operator()( RngPtr rthrd ) const
{
  return static_cast< double >( ldev( rthrd ) );
}
}

#endif

#endif
