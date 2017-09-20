/*
 *  normal_randomdev.cpp
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

#include "normal_randomdev.h"

// C++ includes:
#include <cmath>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

// by default, init as exponential density with mean 1
librandom::NormalRandomDev::NormalRandomDev( RngPtr r_source )
  : RandomDev( r_source )
  , mu_( 0. )
  , sigma_( 1. )
{
}

// threaded
librandom::NormalRandomDev::NormalRandomDev()
  : RandomDev()
  , mu_( 0. )
  , sigma_( 1. )
{
}

void
librandom::NormalRandomDev::set_status( const DictionaryDatum& d )
{
  double new_mu = mu_;
  double new_sigma = sigma_;

  updateValue< double >( d, names::mu, new_mu );
  updateValue< double >( d, names::sigma, new_sigma );

  if ( new_sigma < 0. )
  {
    throw BadParameterValue( "Normal RDV: sigma >= 0 required." );
  }

  mu_ = new_mu;
  sigma_ = new_sigma;
}

void
librandom::NormalRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );

  def< double >( d, names::mu, mu_ );
  def< double >( d, names::sigma, sigma_ );
}

double librandom::NormalRandomDev::operator()( RngPtr r ) const
{
  // Box-Muller algorithm, see Knuth TAOCP, vol 2, 3rd ed, p 122
  // we waste one number
  double V1;
  double V2;
  double S;

  do
  {
    V1 = 2 * r->drand() - 1;
    V2 = 2 * r->drand() - 1;
    S = V1 * V1 + V2 * V2;
  } while ( S >= 1 );
  if ( S != 0 )
  {
    S = V1 * std::sqrt( -2 * std::log( S ) / S );
  }

  return mu_ + sigma_ * S;
}
