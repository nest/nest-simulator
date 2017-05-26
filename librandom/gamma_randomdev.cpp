/*
 *  gamma_randomdev.cpp
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

#include "gamma_randomdev.h"

// C++ includes:
#include <cmath>

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

// by default, init as exponential density with mean 1
librandom::GammaRandomDev::GammaRandomDev( RngPtr r_source, double a_in )
  : RandomDev( r_source )
  , a( a_in )
  , b_( 1.0 )
{
  set_order( a );
}

librandom::GammaRandomDev::GammaRandomDev( double a_in )
  : RandomDev()
  , a( a_in )
  , b_( 1.0 )
{
  set_order( a );
}

double
librandom::GammaRandomDev::unscaled_gamma( RngPtr r ) const
{
  // algorithm depends on order a
  if ( a == 1 )
  {
    return -std::log( r->drandpos() );
  }
  else if ( a < 1 )
  {
    // Johnk's rejection algorithm, see [1], p. 418
    double X;
    double Y;
    double S;
    do
    {
      X = std::pow( r->drand(), ju );
      Y = std::pow( r->drand(), jv );
      S = X + Y;
    } while ( S > 1 );
    if ( X > 0 )
    {
      return -std::log( r->drandpos() ) * X / S;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    // Best's rejection algorithm, see [1], p. 410
    bool accept = false;
    double X = 0.0;
    do
    {
      const double U = r->drand();
      if ( U == 0 or U == 1 )
      {
        continue; // accept guaranteed false
      }
      const double V = r->drand();
      const double W = U * ( 1 - U ); // != 0
      const double Y = std::sqrt( bc / W ) * ( U - 0.5 );
      X = bb + Y;
      if ( X > 0 )
      {
        const double Z = 64 * W * W * W * V * V;
        accept = Z <= 1 - 2 * Y * Y / X;
        if ( not accept )
        {
          accept = std::log( Z ) <= 2 * ( bb * std::log( X / bb ) - Y );
        }
      }
    } while ( not accept );

    return X;
  }
}

void
librandom::GammaRandomDev::set_status( const DictionaryDatum& d )
{
  double a_new = a;
  double b_new = b_;

  updateValue< double >( d, names::order, a_new );
  updateValue< double >( d, names::scale, b_new );

  if ( a_new <= 0. )
  {
    throw BadParameterValue( "Gamma RDV: order > 0 required." );
  }

  if ( b_new <= 0. )
  {
    throw BadParameterValue( "Gamma RDV: scale > 0 required." );
  }

  set_order( a_new );
  b_ = b_new;
}

void
librandom::GammaRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );

  def< double >( d, names::order, a );
  def< double >( d, names::scale, b_ );
}
