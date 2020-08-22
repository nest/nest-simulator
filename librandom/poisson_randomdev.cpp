/*
 *  poisson_randomdev.cpp
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
 *  Implementation based on J H Ahrens, U Dieter, ACM TOMS 8:163-179(1982)
 */

#include "poisson_randomdev.h"

// C++ includes:
#include <algorithm>
#include <climits>
#include <cmath>
#include <limits>

// Includes from libnestutil:
#include "compose.hpp"
#include "numerics.h"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

// Poisson CDF tabulation limit for case mu_ < 10, P(46, 10) ~ eps
const unsigned librandom::PoissonRandomDev::n_tab_ = 46;

// factorials
const unsigned librandom::PoissonRandomDev::fact_[] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880 };

// coefficients for economized polynomial phi(v), see Eq. (6) and Table I
// NOTE: these are not the first 10 coefficients of the series, but the
//       coefficients of the 10th-degree polynomial approximating best
// NOTE: precision is only ~ O(10^-10)
const unsigned librandom::PoissonRandomDev::n_a_ = 10;
const double librandom::PoissonRandomDev::a_[ librandom::PoissonRandomDev::n_a_ ] = { -0.5000000002,
  0.3333333343,
  -0.2499998565,
  0.1999997049,
  -0.1666848753,
  0.1428833286,
  -0.1241963125,
  0.1101687109,
  -0.1142650302,
  0.1055093006 };

librandom::PoissonRandomDev::PoissonRandomDev( RngPtr r_source, double lambda )
  : RandomDev( r_source )
  , mu_( lambda )
  , P_( n_tab_ )
{
  init_();
}

librandom::PoissonRandomDev::PoissonRandomDev( double lambda )
  : RandomDev()
  , mu_( lambda )
  , P_( n_tab_ )
{
  init_();
}

void
librandom::PoissonRandomDev::set_lambda( double lambda )
{
  mu_ = lambda;
  init_();
}

void
librandom::PoissonRandomDev::set_status( const DictionaryDatum& d )
{
  /*
    Limits on mu:

    - mu >= 0 trivial
    - As shown in comments in ldev(), the maximum absolute value
      that can be chosen as a candidate is mu + 710 * sqrt(mu).
    - We thus must require mu + 710 * sqrt(mu) < max(long).
    - This is equivalent to

         mu < ( 2 N + a^2 - sqrt(4 N + a^2) ) / 2

      where N is the largest representable integer and a = 710.
    - Numerical evaluation shows that mu < 0.999 N is safe for 32
      and 64 bit doubles with a good margin.
  */

  const double MU_MAX = 0.999 * std::numeric_limits< long >::max();

  double new_mu = mu_;

  if ( updateValue< double >( d, names::lambda, new_mu ) )
  {
    if ( new_mu < 0 )
    {
      throw BadParameterValue( "Poisson RDV: lambda >= 0 required." );
    }
    if ( new_mu > MU_MAX )
    {
      throw BadParameterValue( String::compose( "Poisson RDV: lambda < %1 required.", MU_MAX ) );
    }
    set_lambda( new_mu );
  }
}

void
librandom::PoissonRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );

  def< double >( d, names::lambda, mu_ );
}

void
librandom::PoissonRandomDev::init_()
{
  assert( mu_ >= 0 );

  if ( mu_ >= 10.0 )
  {

    // case A

    // parameters for steps N, I, S
    s_ = std::sqrt( mu_ );
    d_ = 6 * mu_ * mu_;
    L_ = static_cast< unsigned long >( std::floor( mu_ - 1.1484 ) );

    // parameters for steps P, Q, E, H, F, see Eqs. (12, 13)
    om_ = 1.0 / std::sqrt( 2 * numerics::pi ) / s_;
    double b1_ = 1.0 / ( 24 * mu_ );
    double b2_ = 0.3 * b1_ * b1_;
    c3_ = 1.0 / 7.0 * b1_ * b2_;
    c2_ = b2_ - 15 * c3_;
    c1_ = b1_ - 6 * b2_ + 45 * c3_;
    c0_ = 1 - b1_ + 3 * b2_ - 15 * c3_;

    c_ = 0.1069 / mu_;
  }
  else if ( mu_ > 0.0 )
  {
    // case B

    // tabulate Poisson CDF
    double p = std::exp( -mu_ );
    P_[ 0 ] = p;
    for ( unsigned k = 1; k < n_tab_; ++k )
    {
      p *= mu_ / k;
      // avoid P_[k] > 1.0
      P_[ k ] = std::min( 1.0, P_[ k - 1 ] + p );
    }

    // breaks in case of rounding issues
    assert( ( P_[ n_tab_ - 1 ] <= 1.0 ) && ( 1 - P_[ n_tab_ - 1 ] < 10 * std::numeric_limits< double >::epsilon() ) );

    // ensure table ends with 1.0
    P_[ n_tab_ - 1 ] = 1.0;
  }
  else // mu == 0.0
  {
    P_[ 0 ] = 1.0; // just for safety
  }
}

long
librandom::PoissonRandomDev::ldev( RngPtr r ) const
{
  // the result for lambda == 0 is well defined,
  // added the following two lines of code
  // Diesmann, 26.7.2002
  if ( mu_ == 0.0 )
  {
    return 0;
  }

  unsigned long K = 0; // candidate

  if ( mu_ < 10.0 )
  {
    // Case B in Ahrens & Dieter: table lookup

    double U = ( *r )();

    K = 0; // be defensive
    while ( U > P_[ K ] && K != n_tab_ )
    {
      ++K;
    }

    return K; // maximum value: K == n_tab_ == 46
  }
  else
  {
    // Case A in Ahrens & Dieter

    // Step N ******************************************************

    // draw normal random number
    /* Ratio method (Kinderman-Monahan); see Knuth v2, 3rd ed, p130 */
    /* K+M, ACM Trans Math Software 3 (1977) 257-260. */

    double U, V, T;

    do
    {
      V = ( *r )();
      do
      {
        U = ( *r )();
      } while ( U == 0 );
      /* Const 1.715... = sqrt(8/e) */
      T = 1.71552776992141359295 * ( V - 0.5 ) / U;
    } while ( T * T > -4.0 * std::log( U ) );

    /* maximum for T at this point:

       T*T <= -4 ln U ~ -4 * ln 1e-308 ~ 2837

       => |T| < 54
    */
    double G = mu_ + s_ * T;

    if ( G >= 0 )
    {

      K = static_cast< unsigned long >( std::floor( G ) );

      // Step I ******************************************************
      // immediate acceptance
      if ( K >= L_ )
      {
        return K;
      }

      // Step S ******************************************************
      // squeeze acceptance
      U = ( *r )();
      if ( d_ * U >= std::pow( mu_ - K, 3 ) )
      {
        return K;
      }

      // Step P : see init ******************************************

      // Step Q ****************************************************
      double px, py, fx, fy;
      proc_f_( K, px, py, fx, fy );
      // re-use U from step S, okay since we only apply tighter
      // squeeze criterium
      if ( fy * ( 1 - U ) <= py * std::exp( px - fx ) )
      {
        return K;
      }

      // fall through to E
    }

    // Step E ******************************************************
    double critH;
    do
    {

      double E;

      do
      {
        U = ( *r )();
        E = -std::log( ( *r )() );

        U = U + U - 1;
        T = U >= 0 ? 1.8 + E : 1.8 - E;
      } while ( T <= -0.6744 );

      /* maximum for T at this point:

         0 < E < - ln 1e-308 ~ 709

         => |T| < 710
      */

      // Step H ******************************************************
      K = static_cast< unsigned long >( std::floor( mu_ + s_ * T ) );
      double px, py, fx, fy;
      proc_f_( K, px, py, fx, fy );

      critH = py * std::exp( px + E ) - fy * std::exp( fx + E );

    } while ( c_ * std::abs( U ) > critH );

    return K;

  } // mu < 10
}

void
librandom::PoissonRandomDev::proc_f_( const unsigned K, double& px, double& py, double& fx, double& fy ) const
{
  // Poisson PDF == py * exp(px), see Sec 2

  if ( K < 10 )
  {

    // compute directly

    px = -mu_;
    py = std::pow( mu_, static_cast< int >( K ) ) / fact_[ K ];
  }
  else
  {

    // use Stirling
    double temp = 1.0 / ( 12.0 * K );
    double delta = temp - 4.8 * std::pow( temp, 3 );
    double V = ( mu_ - K ) / static_cast< double >( K );

    if ( std::abs( V ) > 0.25 )
    {

      // cf Eq. (3)
      px = K * std::log( 1 + V ) - ( mu_ - K ) - delta;
    }
    else
    {

      // approximating polynomical, cf Eq. (6)
      // should be converted to Horner form at some point
      px = 0;
      double Vp = 1;
      for ( unsigned j = 0; j < n_a_; ++j )
      {
        px += a_[ j ] * Vp;
        Vp *= V;
      }
      px = px * K * V * V - delta;
    }

    py = 1.0 / std::sqrt( 2 * K * numerics::pi );
  }

  // discrete normal distribution, see Sec. 3

  double x2 = std::pow( ( K - mu_ + 0.5 ) / s_, 2 );

  fx = -x2 / 2; // the minus is present in the FORTRAN code, and in Eq (11)
                // although missing in the pseudocode.

  // cf Eq. (13)
  // NOTE: has only ~ O(10^-8) precision
  fy = om_ * ( ( ( c3_ * x2 + c2_ ) * x2 + c1_ ) * x2 + c0_ );
}
