/*
 *  device.cpp
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

#include "device.h"

// C++ includes:
#include <climits>
#include <limits>

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_names.h"
#include "node.h"

// Includes from sli:
#include "dictutils.h"

/* ----------------------------------------------------------------
 * Default constructor defining default parameters
 * ---------------------------------------------------------------- */

nest::Device::Parameters_::Parameters_()
  : origin_( Time::step( 0 ) )
  , start_( Time::step( 0 ) )
  , stop_( Time::pos_inf() )
{
}

nest::Device::Parameters_::Parameters_( const Parameters_& p )
  : origin_( p.origin_ )
  , start_( p.start_ )
  , stop_( p.stop_ )
{
  /* The resolution of the simulation may have changed since the
     original parameters were set. We thus must calibrate the copies
     to ensure consistency of the time values.
  */
  origin_.calibrate();
  start_.calibrate();
  stop_.calibrate();
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::Device::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::origin ] = origin_.get_ms();
  ( *d )[ names::start ] = start_.get_ms();
  ( *d )[ names::stop ] = stop_.get_ms();
}

void
nest::Device::Parameters_::update_( const DictionaryDatum& d, const Name& name, Time& value )
{
  /* We cannot update the Time values directly, since updateValue()
         doesn't support Time objects. We thus read the value in ms into
         a double first and then update the time object if a value was
         given.

         To be valid, time values must either be on the time grid,
         or be infinite. Infinite values are handled gracefully.
  */

  double val;
  if ( updateValue< double >( d, name, val ) )
  {
    const Time t = Time::ms( val );
    if ( t.is_finite() and not t.is_grid_time() )
    {
      throw BadProperty( name.toString() +  " must be a multiple "
                                 "of the simulation resolution." );
    }
    value = t;
  }
}

void
nest::Device::Parameters_::set( const DictionaryDatum& d )
{
  update_( d, names::origin, origin_ );
  update_( d, names::start, start_ );
  update_( d, names::stop, stop_ );

  if ( stop_ < start_ )
  {
    throw BadProperty( "stop >= start required." );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for device
 * ---------------------------------------------------------------- */

nest::Device::Device()
  : P_()
{
}

nest::Device::Device( const Device& n )
  : P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Device initialization functions
 * ---------------------------------------------------------------- */

void
nest::Device::calibrate()
{
  // We do not need to recalibrate time objects, since they are
  // recalibrated on instance construction and resolution cannot
  // change after a single node instance has been created.

  // by adding time objects, all overflows will be handled gracefully
  V_.t_min_ = ( P_.origin_ + P_.start_ ).get_steps();
  V_.t_max_ = ( P_.origin_ + P_.stop_ ).get_steps();
}
