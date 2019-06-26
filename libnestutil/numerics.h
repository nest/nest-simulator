/*
 *  numerics.h
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

#ifndef NUMERICS_H
#define NUMERICS_H

// Generated includes:
#include "config.h"

// C++ includes:
#include <cassert>
#include <cmath>
#include <limits>

#if HAVE_EXPM1
#include <math.h>
#endif

#if defined( HAVE_STD_ISNAN )
#include <cmath>
#elif defined( HAVE_ISNAN )
#include <math.h>
#endif

namespace numerics
{

extern const double e;
extern const double pi;
extern const double nan;

/** Supply expm1() function independent of system.
 *  @note Implemented inline for efficiency.
 */
inline double
expm1( double x )
{
#if HAVE_EXPM1
  return ::expm1( x ); // use library implementation if available
#else
  // compute using Taylor series, see GSL
  // e^x-1 = x + x^2/2! + x^3/3! + ...
  if ( x == 0 )
  {
    return 0;
  }
  if ( std::abs( x ) > std::log( 2.0 ) )
  {
    return std::exp( x ) - 1;
  }
  else
  {
    double sum = x;
    double term = x * x / 2;
    long n = 2;

    while ( std::abs( term ) > std::abs( sum ) * std::numeric_limits< double >::epsilon() )
    {
      sum += term;
      ++n;
      term *= x / n;
    }

    return sum;
  }
#endif
}

template < typename T >
bool
is_nan( T f )
{
#if defined( HAVE_STD_ISNAN )
  return std::isnan( f );
#elif defined( HAVE_ISNAN )
  return isnan( f );
#else
  assert( false ); // HAVE_STD_ISNAN or HAVE_ISNAN is required
  return false;
#endif
}
}


// later also in namespace
/**
 * Round to nearest int, rounding midpoints upwards.
 *
 * @return Result as long
 * @note [-1/2, 1/2) -> 0 and in general [ (2n-1)/2, (2n+1)/2 ) -> n
 * @see dround
 */
long ld_round( double );

/**
 * Round to nearest int, rounding midpoints upwards.
 *
 * @return Result as double
 * @note [-1/2, 1/2) -> 0 and in general [ (2n-1)/2, (2n+1)/2 ) -> n
 * @see ld_round
 */
double dround( double );

/**
 * Return integer part of argument.
 */
double dtruncate( double );


#endif
