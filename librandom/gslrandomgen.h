/*
 *  gslrandomgen.h
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

/*
 *  Interface to GSL Random Number Generators
 *
 */

#ifndef GSLRANDOMGEN_H
#define GSLRANDOMGEN_H

// C++ includes:
#include <cassert>
#include <list>
#include <string>

// Generated includes:
#include "config.h"

// Includes from librandom:
#include "random_datums.h"
#include "randomgen.h"

// Includes from sli:
#include "dictdatum.h"

// essential GSL includes or replacements
// GSL Versions < 1.2 have weak MT seeding
#ifdef HAVE_GSL

// "Real" version in presence of GSL

// External includes:
#include <gsl/gsl_rng.h>

namespace librandom
{

/**
 * class GslRandomGen
 * C++ wrapper for GSL/GSL-style generators.
 * @note
 * This class should only be used within librandom.
 *
 * @ingroup RandomNumberGenerators
 */

class GslRandomGen : public RandomGen
{
  friend class GSL_BinomialRandomDev;

public:
  explicit GslRandomGen( const gsl_rng_type*, //!< given RNG, given seed
    unsigned long );

  ~GslRandomGen();

  //! Add all GSL RNGs to rngdict
  static void add_gsl_rngs( Dictionary& );

  RngPtr
  clone( unsigned long s )
  {
    return RngPtr( new GslRandomGen( rng_type_, s ) );
  }


private:
  void seed_( unsigned long );
  double drand_( void );

private:
  gsl_rng_type const* rng_type_;
  gsl_rng* rng_;
};

inline void
GslRandomGen::seed_( unsigned long s )
{
  gsl_rng_set( rng_, s );
}

inline double
GslRandomGen::drand_( void )
{
  return gsl_rng_uniform( rng_ );
}

//! Factory class for GSL-based random generators
class GslRNGFactory : public GenericRNGFactory
{
public:
  GslRNGFactory( gsl_rng_type const* const );
  RngPtr create( unsigned long ) const;

private:
  //! GSL generator type information
  gsl_rng_type const* const gsl_rng_;
};
}

#else

// NO GSL Available---Implement class as empty shell
namespace librandom
{

class GslRandomGen : public RandomGen
{
public:
  //! Add all GSL RNGs to rngdict
  //! Do nothing if GSL not available
  static void
  add_gsl_rngs( Dictionary& )
  {
  }

private:
  GslRandomGen()
  {
    assert( false );
  }
  ~GslRandomGen()
  {
    assert( false );
  }
};
}

#endif


#endif
