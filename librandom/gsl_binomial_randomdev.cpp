/*
 *  gsl_binomial_randomdev.cpp
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

#include "gsl_binomial_randomdev.h"

#ifdef HAVE_GSL

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from librandom:
#include "librandom_exceptions.h"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

librandom::GSL_BinomialRandomDev::GSL_BinomialRandomDev( RngPtr r_s, double p_s, unsigned int n_s )
  : RandomDev( r_s )
  , p_( p_s )
  , n_( n_s )
{
  GslRandomGen* gsr_rng = dynamic_cast< GslRandomGen* >( &( *r_s ) );
  if ( not gsr_rng )
  {
    throw UnsuitableRNG( "The gsl_binomial RDV can only be used with GSL RNGs." );
  }
  rng_ = gsr_rng->rng_;
}

librandom::GSL_BinomialRandomDev::GSL_BinomialRandomDev( double p_s, unsigned int n_s )
  : RandomDev()
  , p_( p_s )
  , n_( n_s )
{
}

long
librandom::GSL_BinomialRandomDev::ldev()
{
  return gsl_ran_binomial( rng_, p_, n_ );
}

long
librandom::GSL_BinomialRandomDev::ldev( RngPtr rng ) const
{
  GslRandomGen* gsr_rng = dynamic_cast< GslRandomGen* >( &( *rng ) );
  if ( not gsr_rng )
  {
    throw UnsuitableRNG( "The gsl_binomial RDV can only be used with GSL RNGs." );
  }
  return gsl_ran_binomial( gsr_rng->rng_, p_, n_ );
}

void
librandom::GSL_BinomialRandomDev::set_p_n( double p_s, size_t n_s )
{
  set_p( p_s );
  set_n( n_s );
}

void
librandom::GSL_BinomialRandomDev::set_p( double p_s )
{
  if ( p_s < 0. or 1. < p_s )
  {
    throw BadParameterValue( "gsl_binomial RDV: 0 <= p <= 1 required." );
  }
  p_ = p_s;
}

void
librandom::GSL_BinomialRandomDev::set_n( size_t n_s )
{
  // gsl_ran_binomial() takes n as an unsigned int, so it cannot be greater
  // than what an unsigned int can hold.
  const auto N_MAX = std::numeric_limits< unsigned int >::max();
  if ( n_s >= N_MAX )
  {
    throw BadParameterValue( String::compose( "gsl_binomial RDV: N < %1 required.", static_cast< double >( N_MAX ) ) );
  }
  if ( n_s < 1 )
  {
    throw BadParameterValue( "gsl_binomial RDV: n >= 1 required." );
  }
  n_ = n_s;
}

void
librandom::GSL_BinomialRandomDev::set_status( const DictionaryDatum& d )
{
  double p_new = p_;
  const bool p_updated = updateValue< double >( d, names::p, p_new );

  long n_new = n_;
  const bool n_updated = updateValue< long >( d, names::n, n_new );
  if ( n_new < 1 ) // We must check n_new here in case it is negative
  {
    throw BadParameterValue( "gsl_binomial RDV: n >= 1 required." );
  }
  if ( n_updated or p_updated )
  {
    set_p_n( p_new, n_new );
  }
}

void
librandom::GSL_BinomialRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );

  def< double >( d, names::p, p_ );
  def< long >( d, names::n, n_ );
}

#endif
