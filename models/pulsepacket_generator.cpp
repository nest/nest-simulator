/*
 *  pulsepacket_generator.cpp
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

#include "pulsepacket_generator.h"

// C++ includes:
#include <algorithm>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from librandom:
#include "gslrandomgen.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and variables
 * ---------------------------------------------------------------- */

nest::pulsepacket_generator::Parameters_::Parameters_()
  : pulse_times_()
  , a_( 0 )
  , sdev_( 0.0 )
  , sdev_tolerance_( 10.0 )
{
}

nest::pulsepacket_generator::Variables_::Variables_()
  : start_center_idx_( 0 )
  , stop_center_idx_( 0 )
  , tolerance( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::pulsepacket_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::pulse_times ] = DoubleVectorDatum( new std::vector< double >( pulse_times_ ) );
  ( *d )[ names::activity ] = a_;
  ( *d )[ names::sdev ] = sdev_;
}

void
nest::pulsepacket_generator::Parameters_::set( const DictionaryDatum& d, pulsepacket_generator& ppg, Node* node )
{

  // We cannot use a single line here since short-circuiting may stop evaluation
  // prematurely. Therefore, neednewpulse must be second arg on second line.
  bool neednewpulse = updateValueParam< long >( d, names::activity, a_, node );
  neednewpulse = updateValueParam< double >( d, names::sdev, sdev_, node ) || neednewpulse;
  if ( a_ < 0 )
  {
    throw BadProperty( "The activity cannot be negative." );
  }
  if ( sdev_ < 0 )
  {
    throw BadProperty( "The standard deviation cannot be negative." );
  }


  if ( updateValue< std::vector< double > >( d, "pulse_times", pulse_times_ ) || neednewpulse )
  {
    std::sort( pulse_times_.begin(), pulse_times_.end() );
    ppg.B_.spiketimes_.clear();
  }
}

/* ----------------------------------------------------------------
* Default and copy constructor for node
* ---------------------------------------------------------------- */

nest::pulsepacket_generator::pulsepacket_generator()
  : Node()
  , device_()
  , P_()
{
}

nest::pulsepacket_generator::pulsepacket_generator( const pulsepacket_generator& ppg )
  : Node( ppg )
  , device_( ppg.device_ )
  , P_( ppg.P_ )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::pulsepacket_generator::init_state_( const Node& proto )
{
  const pulsepacket_generator& pr = downcast< pulsepacket_generator >( proto );

  device_.init_state( pr.device_ );
}

void
nest::pulsepacket_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::pulsepacket_generator::calibrate()
{
  device_.calibrate();
  assert( V_.start_center_idx_ <= V_.stop_center_idx_ );

  if ( P_.sdev_ > 0.0 )
  {
    V_.tolerance = P_.sdev_ * P_.sdev_tolerance_;
  }
  else
  {
    V_.tolerance = 1.0;
  }

  const double now = ( kernel().simulation_manager.get_time() ).get_ms();

  V_.start_center_idx_ = 0;
  V_.stop_center_idx_ = 0;


  // determine pulse-center times that lie within
  // a window sdev*sdev_tolerance around the current time
  while (
    V_.stop_center_idx_ < P_.pulse_times_.size() && P_.pulse_times_.at( V_.stop_center_idx_ ) - now <= V_.tolerance )
  {
    if ( std::abs( P_.pulse_times_.at( V_.stop_center_idx_ ) - now ) > V_.tolerance )
    {
      V_.start_center_idx_++;
    }
    V_.stop_center_idx_++;
  }
}


void
nest::pulsepacket_generator::update( Time const& T, const long from, const long to )
{
  assert( to >= from );
  assert( ( to - from ) <= kernel().connection_manager.get_min_delay() );

  if ( ( V_.start_center_idx_ == P_.pulse_times_.size() && B_.spiketimes_.empty() ) || ( not device_.is_active( T ) ) )
  {
    return; // nothing left to do
  }

  // determine next pulse-center times (around sdev*tolerance window)
  if ( V_.stop_center_idx_ < P_.pulse_times_.size() )
  {
    while ( V_.stop_center_idx_ < P_.pulse_times_.size()
      && ( Time( Time::ms( P_.pulse_times_.at( V_.stop_center_idx_ ) ) ) - T ).get_ms() <= V_.tolerance )
    {
      V_.stop_center_idx_++;
    }
  }

  if ( V_.start_center_idx_ < V_.stop_center_idx_ )
  {
    // obtain rng
    librandom::RngPtr rng = kernel().rng_manager.get_rng( get_thread() );

    bool needtosort = false;

    while ( V_.start_center_idx_ < V_.stop_center_idx_ )
    {
      for ( int i = 0; i < P_.a_; i++ )
      {
        double x = P_.sdev_ * V_.norm_dev_( rng ) + P_.pulse_times_.at( V_.start_center_idx_ );
        if ( Time( Time::ms( x ) ) >= T )
        {
          B_.spiketimes_.push_back( Time( Time::ms( x ) ).get_steps() );
        }
      }
      needtosort = true;
      V_.start_center_idx_++;
    }
    if ( needtosort )
    {
      std::sort( B_.spiketimes_.begin(), B_.spiketimes_.end() );
    }
  }

  int n_spikes = 0;

  // Since we have an ordered list of spiketimes,
  // we can compute the histogram on the fly.
  while ( not B_.spiketimes_.empty() && B_.spiketimes_.front() < ( T.get_steps() + to ) )
  {
    n_spikes++;
    long prev_spike = B_.spiketimes_.front();
    B_.spiketimes_.pop_front();

    if ( n_spikes > 0 && prev_spike != B_.spiketimes_.front() )
    {
      SpikeEvent se;
      se.set_multiplicity( n_spikes );
      kernel().event_delivery_manager.send( *this, se, prev_spike - T.get_steps() );
      n_spikes = 0;
    }
  }
}
