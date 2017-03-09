/*
 *  dc_generator.cpp
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

#include "dc_generator.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::dc_generator::Parameters_::Parameters_()
  : amp_( 0.0 ) // pA
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::dc_generator::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::amplitude, amp_ );
}

void
nest::dc_generator::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::amplitude, amp_ );
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::dc_generator::dc_generator()
  : Node()
  , device_()
  , P_()
{
}

nest::dc_generator::dc_generator( const dc_generator& n )
  : Node( n )
  , device_( n.device_ )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::dc_generator::init_state_( const Node& proto )
{
  const dc_generator& pr = downcast< dc_generator >( proto );

  device_.init_state( pr.device_ );
}

void
nest::dc_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::dc_generator::calibrate()
{
  device_.calibrate();
}


/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::dc_generator::update( Time const& origin, const long from, const long to )
{
  long start = origin.get_steps();

  CurrentEvent ce;
  ce.set_current( P_.amp_ );
  for ( long offs = from; offs < to; ++offs )
  {
    if ( device_.is_active( Time::step( start + offs ) ) )
    {
      kernel().event_delivery_manager.send( *this, ce, offs );
    }
  }
}
