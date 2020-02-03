/*
 *  binomial_randomdev.cpp
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


/* ----------------------------------------------------------------
 * Draw a binomial random number using the BP algoritm
 * Sampling From the Binomial Distribution on a Computer
 * Author(s): George S. Fishman
 * Source: Journal of the American Statistical Association, Vol. 74, No. 366
 *         (Jun., 1979), pp. 418-423
 * Published by: American Statistical Association
 * Stable URL: http://www.jstor.org/stable/2286346 .
 * ---------------------------------------------------------------- */


#include "binomial_randomdev.h"

// C++ includes:
#include <cmath>
#include <limits>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from sli:
#include "dictutils.h"

librandom::BinomialRandomDev::BinomialRandomDev( RngPtr r_s, double p_s, unsigned int n_s )
  : RandomDev( r_s )
  , poisson_dev_( r_s )
  , exp_dev_( r_s )
  , p_( p_s )
  , n_( n_s )
{
  init_();
  PrecomputeTable( n_s );
}

librandom::BinomialRandomDev::BinomialRandomDev( double p_s, unsigned int n_s )
  : RandomDev()
  , poisson_dev_()
  , exp_dev_()
  , p_( p_s )
  , n_( n_s )
{
  init_();
  PrecomputeTable( n_s );
}


void
librandom::BinomialRandomDev::PrecomputeTable( size_t nmax )
{
  // precompute the table of f
  f_.resize( nmax + 2 );
  f_[ 0 ] = 0.0;
  f_[ 1 ] = 0.0;
  unsigned long i, j;
  i = 1;
  while ( i < f_.size() - 1 )
  {
    f_[ i + 1 ] = 0.0;
    j = 1;
    while ( j <= i )
    {
      f_[ i + 1 ] += std::log( static_cast< double >( j ) );
      j++;
    }
    i++;
  }
  n_tablemax_ = nmax;
}


long
librandom::BinomialRandomDev::ldev( RngPtr rng ) const
{
  // BP algorithm (steps numbered as in Fishman 1979)
  // Steps 1-7 are in init_()
  unsigned long X;
  double V;
  long Y;

  bool not_finished = 1;
  while ( not_finished )
  {
    // 8,9
    X = n_ + 1;
    while ( X > n_ )
    {
      X = poisson_dev_.ldev( rng );
    }

    // 10
    V = exp_dev_( rng );

    // 11
    Y = n_ - X;

    // 12
    if ( V < static_cast< double >( m_ - Y ) * phi_ - f_[ m_ + 1 ] + f_[ Y + 1 ] )
    {
      not_finished = 1;
    }
    else
    {
      not_finished = 0;
    }
  }
  if ( p_ <= 0.5 )
  {
    return X;
  }
  else
  {
    return static_cast< unsigned long >( Y );
  }
}


void
librandom::BinomialRandomDev::set_p_n( double p_s, unsigned int n_s )
{
  p_ = p_s;
  n_ = n_s;
  init_();
  if ( n_s > n_tablemax_ )
  {
    PrecomputeTable( n_s );
  }
}

void
librandom::BinomialRandomDev::set_p( double p_s )
{
  p_ = p_s;
  init_();
}

void
librandom::BinomialRandomDev::set_n( unsigned int n_s )
{
  n_ = n_s;
  init_();
  if ( n_s > n_tablemax_ )
  {
    PrecomputeTable( n_s );
  }
}

void
librandom::BinomialRandomDev::init_()
{
  assert( 0.0 <= p_ && p_ <= 1.0 );

  double q, mu;

  // 1, 2
  if ( p_ > 0.5 )
  {
    q = 1. - p_;
  }
  else
  {
    q = p_;
  }

  // 3,4
  long n1mq = static_cast< long >( static_cast< double >( n_ ) * ( 1. - q ) );
  double n1mq_dbl = static_cast< double >( n1mq );
  if ( static_cast< double >( n_ ) * ( 1. - q ) - n1mq_dbl > q )
  {
    mu = q * ( n1mq_dbl + 1. ) / ( 1. - q );
  }
  else
  {
    mu = static_cast< double >( n_ ) - n1mq_dbl;
  }

  // 5, 6, 7
  double theta = ( 1. / q - 1. ) * mu;
  phi_ = std::log( theta );
  m_ = static_cast< long >( theta );
  poisson_dev_.set_lambda( mu );
}

void
librandom::BinomialRandomDev::set_status( const DictionaryDatum& d )
{
  double p_new = p_;
  const bool p_updated = updateValue< double >( d, names::p, p_new );

  long n_new = n_;
  const bool n_updated = updateValue< long >( d, names::n, n_new );

  if ( p_new < 0. || 1. < p_new )
  {
    throw BadParameterValue( "Binomial RDV: 0 <= p <= 1 required." );
  }
  if ( n_new < 1 )
  {
    throw BadParameterValue( "Binomial RDV: n >= 1 required." );
  }

  // Binomial numbers are generated from Poisson numbers.
  // To avoid an infinite loop, we limit n to slightly less than
  // the maximum possible value for Poisson numbers
  const long N_MAX = static_cast< long >( 0.998 * std::numeric_limits< long >::max() );
  if ( n_new > N_MAX )
  {
    throw BadParameterValue( String::compose( "Binomial RDV: N < %1 required.", static_cast< double >( N_MAX ) ) );
  }
  if ( n_updated || p_updated )
  {
    set_p_n( p_new, n_new );
  }
}

void
librandom::BinomialRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );
  def< double >( d, names::p, p_ );
  def< long >( d, names::n, n_ );
}
