/*
 *  delay_types.h
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

#ifndef DELAY_TYPES_H
#define DELAY_TYPES_H

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_time.h"
#include "nest_types.h"

namespace nest
{


/**
 * Container for a single delay value which is interpreted as total transmission delay or purely dendritic delay by some
 * models. Implies an axonal delay of 0.
 */
struct TotalDelay
{
  uint32_t delay_;

  explicit TotalDelay( double d )
  {
    set_delay_ms( d );
  }

  /**
   * Return the dendritic delay of the connection in steps
   */
  long
  get_dendritic_delay_steps() const
  {
    throw BadProperty( "Trying to get dendritic delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Set the dendritic delay of the connection specified in ms
   */
  void
  set_dendritic_delay_steps( const double )
  {
    throw BadProperty( "Trying to set dendritic delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Return the dendritic delay of the connection in ms
   */
  double
  get_dendritic_delay_ms() const
  {
    throw BadProperty( "Trying to get dendritic delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Set the dendritic delay of the connection specified in ms
   */
  void
  set_dendritic_delay_ms( const double )
  {
    throw BadProperty( "Trying to set dendritic delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Return the axonal delay of the connection in steps
   */
  long
  get_axonal_delay_steps() const
  {
    throw BadProperty( "Trying to get axonal delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Set the axonal delay of the connection specified in ms
   */
  void
  set_axonal_delay_steps( const double )
  {
    throw BadProperty( "Trying to set axonal delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Return the axonal delay of the connection in ms
   */
  double
  get_axonal_delay_ms() const
  {
    throw BadProperty( "Trying to get axonal delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Set the axonal delay of the connection specified in ms
   */
  void
  set_axonal_delay_ms( const double )
  {
    throw BadProperty( "Trying to set axonal delay on a synapse which only stores the total transmission delay." );
  }

  /**
   * Return the delay of the connection in steps
   */
  long
  get_delay_steps() const
  {
    return delay_;
  }

  /**
   * Set the delay of the connection specified in ms
   */
  void
  set_delay_steps( const long d )
  {
    delay_ = d;
  }

  /**
   * Return the delay of the connection in ms
   */
  double
  get_delay_ms() const
  {
    return Time::delay_steps_to_ms( delay_ );
  }

  /**
   * Set the delay of the connection specified in ms
   */
  void
  set_delay_ms( const double d )
  {
    delay_ = Time::delay_ms_to_steps( d );
  }

  void
  calibrate( const TimeConverter& tc )
  {
    Time t = tc.from_old_steps( delay_ );
    delay_ = t.get_steps();

    if ( delay_ == 0 )
    {
      delay_ = 1;
    }
  }

  void
  get_status( DictionaryDatum& d ) const
  {
    def< double >( d, names::delay, Time::delay_steps_to_ms( delay_ ) );
  }

  void
  set_status( const DictionaryDatum& d, ConnectorModel& )
  {
    // Check for allowed combinations. See PR #2989 for more details.
    if ( d->known( names::dendritic_delay ) or d->known( names::axonal_delay ) )
    {
      throw BadParameter( "Synapse type does not support explicitly setting axonal and dendritic delays." );
    }

    // Update delay values
    double delay;
    if ( updateValue< double >( d, names::delay, delay ) )
    {
      set_delay_ms( delay );
    }

    kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( get_delay_ms() );
  }
};

//! check legal size
using success_total_transmission_delay_data_size = StaticAssert< sizeof( TotalDelay ) == 4 >::success;

/**
 * Container for explicit dendritic and axonal delay values.
 */
struct AxonalDendriticDelay
{
  uint32_t dendritic_delay_ : NUM_BITS_DENDRITIC_DELAY;
  uint32_t axonal_delay_ : NUM_BITS_AXONAL_DELAY;

  explicit AxonalDendriticDelay( double d )
    : axonal_delay_( 0 )
  {
    set_dendritic_delay_ms( d );
  }

  /**
   * Return the dendritic delay of the connection in steps
   */
  long
  get_dendritic_delay_steps() const
  {
    return dendritic_delay_;
  }

  /**
   * Set the dendritic delay of the connection specified in ms
   */
  void
  set_dendritic_delay_steps( const long d )
  {
    dendritic_delay_ = d;
  }

  /**
   * Return the dendritic delay of the connection in ms
   */
  double
  get_dendritic_delay_ms() const
  {
    return Time::delay_steps_to_ms( dendritic_delay_ );
  }

  /**
   * Set the dendritic delay of the connection specified in ms
   */
  void
  set_dendritic_delay_ms( const double d )
  {
    dendritic_delay_ = Time::delay_ms_to_steps( d );
  }

  /**
   * Return the axonal delay of the connection in steps
   */
  long
  get_axonal_delay_steps() const
  {
    return axonal_delay_;
  }

  /**
   * Set the axonal delay of the connection specified in ms
   */
  void
  set_axonal_delay_steps( const long d )
  {
    axonal_delay_ = d;
  }

  /**
   * Return the axonal delay of the connection in ms
   */
  double
  get_axonal_delay_ms() const
  {
    return Time::delay_steps_to_ms( axonal_delay_ );
  }

  /**
   * Set the axonal delay of the connection specified in ms
   */
  void
  set_axonal_delay_ms( const double d )
  {
    axonal_delay_ = Time::delay_ms_to_steps( d );
  }

  /**
   * Return the delay of the connection in steps
   */
  long
  get_delay_steps() const
  {
    return dendritic_delay_ + axonal_delay_;
  }

  /**
   * Set the delay of the connection specified in ms
   */
  void
  set_delay_steps( const long d )
  {
    dendritic_delay_ = d;
    axonal_delay_ = 0;
  }

  /**
   * Return the delay of the connection in ms
   */
  double
  get_delay_ms() const
  {
    return Time::delay_steps_to_ms( get_delay_steps() );
  }

  /**
   * Set the delay of the connection specified in ms
   */
  void
  set_delay_ms( const double d )
  {
    dendritic_delay_ = Time::delay_ms_to_steps( d );
    axonal_delay_ = 0;
  }

  void
  calibrate( const TimeConverter& tc )
  {
    Time ax_delay_t = tc.from_old_steps( axonal_delay_ );
    Time dend_delay_t = tc.from_old_steps( dendritic_delay_ );
    axonal_delay_ = ax_delay_t.get_steps();
    dendritic_delay_ = dend_delay_t.get_steps();

    if ( dendritic_delay_ == 0 )
    {
      dendritic_delay_ = 1;
    }
  }

  void
  get_status( DictionaryDatum& d ) const
  {
    def< double >( d, names::dendritic_delay, Time::delay_steps_to_ms( dendritic_delay_ ) );
    def< double >( d, names::axonal_delay, Time::delay_steps_to_ms( axonal_delay_ ) );
    def< double >( d, names::delay, Time::delay_steps_to_ms( axonal_delay_ + dendritic_delay_ ) );
  }

  void
  set_status( const DictionaryDatum& d, ConnectorModel& )
  {
    if ( d->known( names::delay ) )
    {
      throw BadParameter( "Setting the total transmission delay via the parameter '" + names::delay.toString()
        + "' is not allowed for synapse types which use both dendritic and axonal delays, because of ambiguity." );
    }

    // Update delay values
    double dendritic_delay = get_dendritic_delay_ms();
    if ( updateValue< double >( d, names::dendritic_delay, dendritic_delay ) )
    {
      set_dendritic_delay_ms( dendritic_delay );
    }
    double axonal_delay = get_axonal_delay_ms();
    if ( updateValue< double >( d, names::axonal_delay, axonal_delay ) )
    {
      set_axonal_delay_ms( axonal_delay );
    }

    kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( get_delay_ms() );
  }
};

//! check legal size
using success_axonal_dendritic_delay_data_size = StaticAssert< sizeof( AxonalDendriticDelay ) == 4 >::success;

}

#endif
