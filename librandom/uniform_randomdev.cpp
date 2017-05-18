/*
 *  uniform_randomdev.cpp
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

#include "uniform_randomdev.h"

// C++ includes:
#include <cmath>

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

// by default, init with interval [0, 1)
librandom::UniformRandomDev::UniformRandomDev( RngPtr r_source )
  : RandomDev( r_source )
  , low_( 0 )
  , high_( 1 )
  , delta_( high_ - low_ )
{
}

librandom::UniformRandomDev::UniformRandomDev()
  : RandomDev()
  , low_( 0 )
  , high_( 1 )
  , delta_( high_ - low_ )
{
}

void
librandom::UniformRandomDev::set_status( const DictionaryDatum& d )
{
  double new_low = low_;
  double new_high = high_;

  updateValue< double >( d, names::low, new_low );
  updateValue< double >( d, names::high, new_high );
  if ( new_high <= new_low )
  {
    throw BadParameterValue( "Uniform RDV: low < high required." );
  }

  low_ = new_low;
  high_ = new_high;
  delta_ = high_ - low_;
}

void
librandom::UniformRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );

  def< double >( d, names::low, low_ );
  def< double >( d, names::high, high_ );
}
