/*
 *  numerics.cpp
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

#include <cstdlib>
#include <iostream>
#include <numeric>

#include "nest_types.h"
#include "numerics.h"

#ifndef HAVE_M_E

#ifdef HAVE_CMATH_MAKROS_IGNORED
#define M_E_OK
#undef __PURE_CNAME
#include <cmath>
#define __PURE_CNAME
#else
#include <cmath>
#endif

#else
#define M_E_OK
#include <cmath>
#endif


#ifndef HAVE_M_PI

#ifdef HAVE_CMATH_MAKROS_IGNORED
#define M_PI_OK
#endif

#else
#define M_PI_OK
#endif

#if defined( HAVE_STD_NAN )
#include <cmath>
#elif defined( HAVE_NAN )
#include <math.h>
#endif

//
//   e
//
#ifdef HAVE_GSL

#include <gsl/gsl_math.h>
const double numerics::e = M_E;
const double numerics::pi = M_PI;

#else

#ifdef M_E_OK
const double numerics::e = M_E;
#else
const double numerics::e = 2.71828182845904523536028747135;
#endif

#ifdef M_PI_OK
const double numerics::pi = M_PI;
#else
const double numerics::pi = 3.14159265358979323846264338328;
#endif

#endif

#if defined( HAVE_STD_NAN )
const double numerics::nan = ::nan( "" );
#elif defined( HAVE_NAN )
const double numerics::nan = NAN;
#else
const double numerics::nan = 0.0 / 0.0;
#endif

const double numerics::sqrt_log_two = std::sqrt( std::log( 2.0 ) );

// later also in namespace
long
ld_round( double x )
{
  return static_cast< long >( std::floor( x + 0.5 ) );
}

double
dround( double x )
{
  return std::floor( x + 0.5 );
}

double
dtruncate( double x )
{
  double ip;

  std::modf( x, &ip );
  return ip;
}

bool
is_integer( double n )
{
  double int_part;
  double frac_part = std::modf( n, &int_part );

  // Since n > 0 and modf always rounds towards zero, a value of n just below an
  // integer will result in frac_part = 0.99999.... . We subtract from 1 in this case.
  if ( frac_part > 0.5 )
  {
    frac_part = 1 - frac_part;
  }

  // factor 4 allows for two bits of rounding error
  return frac_part < 4 * n * std::numeric_limits< double >::epsilon();
}

long
mod_inverse( long a, long m )
{
  /*
   The implementation here is based on the extended Euclidean algorithm which
   solves

     a x + m y = gcd( a, m ) = 1 mod m

   for x and y. Note that the m y term is zero mod m, so the equation is equivalent
   to

     a x = 1 mod m

   Since we only need x, we can ignore y and use just half of the algorithm.

   We can assume without loss of generality that a < m, because if a = a' + j m with
   0 < a' < m, we have

      a x mod m = (a' + j m) x mod m = a' x + j x m mod m = a' x.

   This implies that m ≥ 2.

   For details on the algorithm, see D. E. Knuth, The Art of Computer Programming,
   ch 4.5.2, Algorithm X (vol 2), and ch 1.2.1, Algorithm E (vol 1).
   */

  assert( 0 < a );
  assert( 2 <= m );

  const long a_orig = a;
  const long m_orig = m;

  // If a ≥ m, the algorithm needs two extra rounds to transform this to
  // a' < m, so we take care of this in a single step here.
  a = a % m;

  // Use half of extended Euclidean algorithm required to compute inverse
  long s_0 = 1;
  long s_1 = 0;

  while ( a > 0 )
  {
    // get quotient and remainder in one go
    const auto res = div( m, a );
    m = a;
    a = res.rem;

    // line ordering matters here
    const long s_0_new = -res.quot * s_0 + s_1;
    s_1 = s_0;
    s_0 = s_0_new;
  }

  // ensure positive result
  s_1 = ( s_1 + m_orig ) % m_orig;

  assert( m == 1 );                         // gcd() == 1 required
  assert( ( a_orig * s_1 ) % m_orig == 1 ); // self-test

  return s_1;
}

size_t
first_index( long period, long phase0, long step, long phase )
{
  assert( period > 0 );
  assert( step > 0 );
  assert( 0 <= phase0 and phase0 < period );
  assert( 0 <= phase and phase < period );

  /*
   The implementation here is based on
   https://math.stackexchange.com/questions/25390/how-to-find-the-inverse-modulo-m

   We first need to solve

        phase0 + k step = phase mod period
   <=>  k step = ( phase - phase0 ) = d_phase mod period

   This has a solution iff d = gcd(step, period) divides d_phase.

   Then, if d = 1, the solution is unique and given by

        k' = mod_inv(step) * d_phase mod period

   If d > 1, we need to divide the equation by it and solve

        (step / d) k_0 = d_phase / d  mod (period / d)

   The set of solutions is then given by

        k_j = k_0 + j * period / d  for j = 0, 1, ..., d-1

   and we need the smallest of these. Now we are interested in
   an index given by k * step with a period of lcm(step, period)
   (the outer_period below, marked by | in the illustration in
   the doxygen comment), we immediately run over

        k_j * step = k_0 * step + j * step * period / d mod lcm(step, period)

   below and take the smallest. But since step * period / d = lcm(step, period),
   the term in j above vanishes and k_0 * step mod lcm(step, period) is actually
   the solution.

   We do all calculations in signed long since we may encounter negative
   values during the algorithm. The result will be non-negative and returned
   as size_t. This is important because the "not found" case is signaled
   by invalid_index, which is size_t.
   */

  // This check is not only a convenience: If step == k * period, we only match if
  // phase == phase0 and the algorithm below will fail if we did not return here
  // immediately, because we get d == period -> period_d = 1 and modular inverse
  // for modulus 1 makes no sense.
  if ( phase == phase0 )
  {
    return 0;
  }

  const long d_phase = ( phase - phase0 + period ) % period;
  const long d = std::gcd( step, period );

  if ( d_phase % d != 0 )
  {
    return nest::invalid_index; // no solution exists
  }

  // Scale by GCD, since modular inverse requires gcd==1
  const long period_d = period / d;
  const long step_d = step / d;
  const long d_phase_d = d_phase / d;

  // Compute k_0 and multiply by step, see explanation in introductory comment
  const long idx = ( d_phase_d * mod_inverse( step_d, period_d ) * step ) % std::lcm( period, step );

  return static_cast< size_t >( idx );
}
