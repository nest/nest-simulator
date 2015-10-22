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

#include <algorithm> // min, max

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
  ( *d )[ "min_delay" ] = get_min_delay().get_ms();
  ( *d )[ "max_delay" ] = get_max_delay().get_ms();
}

void
nest::DelayChecker::set_status( const DictionaryDatum& d )
{
  /*
   * In the following code, we do not round delays to steps. For min and max delay,
   * this is not strictly necessary. For a newly set delay, the rounding will be
   * handled in cp_.set_status() or default_connection_.set_status().
   * Since min_/max_delay are Time-objects and comparison is defined on Time
   * objects, we should use it.
   */
  Time min_delay, max_delay, new_delay;
  double_t delay_tmp;
  bool min_delay_updated = updateValue< double_t >( d, "min_delay", delay_tmp );
  min_delay = Time( Time::ms( delay_tmp ) );
  bool max_delay_updated = updateValue< double_t >( d, "max_delay", delay_tmp );
  max_delay = Time( Time::ms( delay_tmp ) );

  if ( min_delay_updated xor max_delay_updated )
    LOG( M_ERROR, "SetDefaults", "Both min_delay and max_delay have to be specified" );

  if ( min_delay_updated && max_delay_updated )
  {
    if ( kernel().connection_builder_manager.get_num_connections() > 0 )
      LOG( M_ERROR, "SetDefaults", "Connections already exist. Please call ResetKernel first" );
    else if ( min_delay < Time::get_resolution() )
      LOG( M_ERROR, "SetDefaults", "min_delay must be greater than or equal to resolution" );
    else if ( max_delay < Time::get_resolution() )
      LOG( M_ERROR, "SetDefaults", "max_delay must be greater than or equal to resolution" );
    else
    {
      min_delay_ = min_delay;
      max_delay_ = max_delay;
      user_set_delay_extrema_ = true;
    }
  }
}

void
nest::DelayChecker::update_delay_extrema( const double_t mindelay_cand,
  const double_t maxdelay_cand )
{
  if ( not freeze_delay_update_ )
  {
    // check min delay candidate
    Time delay_cand = Time( Time::ms( mindelay_cand ) );
    if ( delay_cand < min_delay_ )
      min_delay_ = delay_cand;

    // check max delay candidate
    delay_cand = Time( Time::ms( maxdelay_cand ) );
    if ( delay_cand > max_delay_ )
      max_delay_ = delay_cand;
  }
}

void
nest::DelayChecker::assert_valid_delay_ms( double_t requested_new_delay )
{
  // We have to convert the delay in ms to a Time object then to steps and back the ms again
  // in order to get the value in ms which can be represented with an integer number of steps
  // in the currently chosen Time representation.
  // See also bug #217, MH 08-04-23
  // This is also done by creating a Time object out of the provided ms.
  const Time new_delay = Time( Time::ms( requested_new_delay ) );

  if ( new_delay < Time::get_resolution() )
    throw BadDelay( new_delay.get_ms(), "Delay must be greater than or equal to resolution" );

  // if already simulated, the new delay has to be checked against the
  // min_delay and the max_delay which have been used during simulation
  if ( kernel().simulation_manager.has_been_simulated() )
  {
    Time sim_min_delay = Time::step( kernel().connection_builder_manager.get_min_delay() );
    Time sim_max_delay = Time::step( kernel().connection_builder_manager.get_max_delay() );
    const bool bad_min_delay = new_delay < sim_min_delay;
    const bool bad_max_delay = new_delay > sim_max_delay;

    if ( bad_min_delay || bad_max_delay )
      throw BadDelay( new_delay.get_ms(),
        "Minimum and maximum delay cannot be changed after Simulate has been called." );
  }

  const bool new_min_delay = new_delay < min_delay_;
  const bool new_max_delay = new_delay > max_delay_;

  if ( new_min_delay )
  {
    if ( user_set_delay_extrema_ )
      throw BadDelay( new_delay.get_ms(), "Delay must be greater than or equal to min_delay." );
    else
      update_delay_extrema( new_delay.get_ms(), max_delay_.get_ms() );
  }

  if ( new_max_delay )
  {
    if ( user_set_delay_extrema_ )
      throw BadDelay( new_delay.get_ms(), "Delay must be smaller than or equal to max_delay." );
    else
      update_delay_extrema( min_delay_.get_ms(), new_delay.get_ms() );
  }
}

void
nest::DelayChecker::assert_two_valid_delays_steps( long_t new_delay1, long_t new_delay2 )
{
  const long_t ldelay = std::min( new_delay1, new_delay2 );
  const long_t hdelay = std::max( new_delay1, new_delay2 );

  if ( ldelay < Time::get_resolution().get_steps() )
    throw BadDelay(
      Time::delay_steps_to_ms( ldelay ), "Delay must be greater than or equal to resolution" );

  if ( kernel().simulation_manager.has_been_simulated() )
  {
    const bool bad_min_delay = ldelay < kernel().connection_builder_manager.get_min_delay();
    const bool bad_max_delay = hdelay > kernel().connection_builder_manager.get_max_delay();

    if ( bad_min_delay )
      throw BadDelay( Time::delay_steps_to_ms( ldelay ),
        "Minimum delay cannot be changed after Simulate has been called." );

    if ( bad_max_delay )
      throw BadDelay( Time::delay_steps_to_ms( hdelay ),
        "Maximum delay cannot be changed after Simulate has been called." );
  }

  const bool new_min_delay = ldelay < min_delay_.get_steps();
  const bool new_max_delay = hdelay > max_delay_.get_steps();

  if ( new_min_delay )
  {
    if ( user_set_delay_extrema_ )
      throw BadDelay(
        Time::delay_steps_to_ms( ldelay ), "Delay must be greater than or equal to min_delay." );
    else
      update_delay_extrema( Time::delay_steps_to_ms( ldelay ), max_delay_.get_ms() );
  }

  if ( new_max_delay )
  {
    if ( user_set_delay_extrema_ )
      throw BadDelay(
        Time::delay_steps_to_ms( hdelay ), "Delay must be smaller than or equal to max_delay." );
    else
      update_delay_extrema( min_delay_.get_ms(), Time::delay_steps_to_ms( hdelay ) );
  }
}