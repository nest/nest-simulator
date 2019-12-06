/*
 *  delay_checker.cpp
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

#include "delay_checker.h"

// C++ includes:
#include <algorithm> // min, max

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_timeconverter.h"

nest::DelayChecker::DelayChecker()
  : min_delay_( Time::pos_inf() )
  , max_delay_( Time::neg_inf() )
  , user_set_delay_extrema_( false )
  , freeze_delay_update_( false )
{
}

nest::DelayChecker::DelayChecker( const DelayChecker& cr )
  : min_delay_( cr.min_delay_ )
  , max_delay_( cr.max_delay_ )
  , user_set_delay_extrema_( cr.user_set_delay_extrema_ )
  , freeze_delay_update_( cr.freeze_delay_update_ )
{
  min_delay_.calibrate(); // in case of change in resolution
  max_delay_.calibrate();
}

void
nest::DelayChecker::calibrate( const TimeConverter& tc )
{
  // Calibrate will be called after a change in resolution, when there are no
  // network elements present.
  min_delay_ = tc.from_old_tics( min_delay_.get_tics() );
  max_delay_ = tc.from_old_tics( max_delay_.get_tics() );
}

void
nest::DelayChecker::get_status( DictionaryDatum& d ) const
{
  ( *d )[ names::min_delay ] = get_min_delay().get_ms();
  ( *d )[ names::max_delay ] = get_max_delay().get_ms();
}

void
nest::DelayChecker::set_status( const DictionaryDatum& d )
{
  // For the minimum delay, we always round down. The easiest way to do this,
  // is to round up and then subtract one step. The only remaining edge case
  // is that the min delay is exactly at a step, in which case one would get
  // a min delay that is one step too small. We can detect this by an
  // additional test.
  double delay_tmp = 0.0;
  bool min_delay_updated = updateValue< double >( d, names::min_delay, delay_tmp );
  Time new_min_delay;
  if ( min_delay_updated )
  {
    delay new_min_delay_steps = Time( Time::ms_stamp( delay_tmp ) ).get_steps();
    if ( Time( Time::step( new_min_delay_steps ) ).get_ms() > delay_tmp )
    {
      new_min_delay_steps -= 1;
    }
    new_min_delay = Time( Time::step( new_min_delay_steps ) );
  }

  // For the maximum delay, we always round up, using ms_stamp
  bool max_delay_updated = updateValue< double >( d, names::max_delay, delay_tmp );
  Time new_max_delay = Time( Time::ms_stamp( delay_tmp ) );

  if ( min_delay_updated xor max_delay_updated )
  {
    throw BadProperty( "Both min_delay and max_delay have to be specified" );
  }

  if ( min_delay_updated and max_delay_updated )
  {
    if ( kernel().connection_manager.get_num_connections() > 0 )
    {
      throw BadProperty( "Connections already exist. Please call ResetKernel first" );
    }
    else if ( new_min_delay < Time::get_resolution() )
    {
      throw BadDelay( new_min_delay.get_ms(), "min_delay must be greater than or equal to resolution." );
    }
    else if ( new_max_delay < new_min_delay )
    {
      throw BadDelay( new_min_delay.get_ms(), "min_delay must be smaller than or equal to max_delay." );
    }
    else
    {
      min_delay_ = new_min_delay;
      max_delay_ = new_max_delay;
      user_set_delay_extrema_ = true;
    }
  }
}

void
nest::DelayChecker::assert_valid_delay_ms( double requested_new_delay )
{
  const delay new_delay = Time::delay_ms_to_steps( requested_new_delay );
  const double new_delay_ms = Time::delay_steps_to_ms( new_delay );

  if ( new_delay < Time::get_resolution().get_steps() )
  {
    throw BadDelay( new_delay_ms, "Delay must be greater than or equal to resolution" );
  }

  // if already simulated, the new delay has to be checked against the
  // min_delay and the max_delay which have been used during simulation
  if ( kernel().simulation_manager.has_been_simulated() )
  {
    const bool bad_min_delay = new_delay < kernel().connection_manager.get_min_delay();
    const bool bad_max_delay = new_delay > kernel().connection_manager.get_max_delay();
    if ( bad_min_delay or bad_max_delay )
    {
      throw BadDelay( new_delay_ms,
        "Minimum and maximum delay cannot be changed "
        "after Simulate has been called." );
    }
  }

  const bool new_min_delay = new_delay < min_delay_.get_steps();
  const bool new_max_delay = new_delay > max_delay_.get_steps();

  if ( new_min_delay )
  {
    if ( user_set_delay_extrema_ )
    {
      throw BadDelay( new_delay_ms,
        "Delay must be greater than or equal to min_delay. "
        "You may set min_delay before creating connections." );
    }
    else
    {
      if ( not freeze_delay_update_ )
      {
        min_delay_ = Time( Time::step( new_delay ) );
      }
    }
  }

  if ( new_max_delay )
  {
    if ( user_set_delay_extrema_ )
    {
      throw BadDelay( new_delay_ms,
        "Delay must be smaller than or equal to max_delay. "
        "You may set min_delay before creating connections." );
    }
    else
    {
      if ( not freeze_delay_update_ )
      {
        max_delay_ = Time( Time::step( new_delay ) );
      }
    }
  }
}

void
nest::DelayChecker::assert_two_valid_delays_steps( delay new_delay1, delay new_delay2 )
{
  const delay ldelay = std::min( new_delay1, new_delay2 );
  const delay hdelay = std::max( new_delay1, new_delay2 );

  if ( ldelay < Time::get_resolution().get_steps() )
  {
    throw BadDelay( Time::delay_steps_to_ms( ldelay ), "Delay must be greater than or equal to resolution" );
  }

  if ( kernel().simulation_manager.has_been_simulated() )
  {
    const bool bad_min_delay = ldelay < kernel().connection_manager.get_min_delay();
    const bool bad_max_delay = hdelay > kernel().connection_manager.get_max_delay();
    if ( bad_min_delay )
    {
      throw BadDelay(
        Time::delay_steps_to_ms( ldelay ), "Minimum delay cannot be changed after Simulate has been called." );
    }
    if ( bad_max_delay )
    {
      throw BadDelay(
        Time::delay_steps_to_ms( hdelay ), "Maximum delay cannot be changed after Simulate has been called." );
    }
  }

  const bool new_min_delay = ldelay < min_delay_.get_steps();
  const bool new_max_delay = hdelay > max_delay_.get_steps();

  if ( new_min_delay )
  {
    if ( user_set_delay_extrema_ )
    {
      throw BadDelay( Time::delay_steps_to_ms( ldelay ),
        "Delay must be greater than or equal to min_delay. "
        "You may set min_delay before creating connections." );
    }
    else
    {
      if ( not freeze_delay_update_ )
      {
        min_delay_ = Time( Time::step( ldelay ) );
      }
    }
  }

  if ( new_max_delay )
  {
    if ( user_set_delay_extrema_ )
    {
      throw BadDelay( Time::delay_steps_to_ms( hdelay ),
        "Delay must be smaller than or equal to max_delay. "
        "You may set max_delay before creating connections." );
    }
    else
    {
      if ( not freeze_delay_update_ )
      {
        max_delay_ = Time( Time::step( hdelay ) );
      }
    }
  }
}
