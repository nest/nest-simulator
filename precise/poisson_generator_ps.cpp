/*
 *  poisson_generator_ps.cpp
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

#include "poisson_generator_ps.h"

// C++ includes:
#include <algorithm>
#include <limits>

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::poisson_generator_ps::Parameters_::Parameters_()
  : rate_( 0.0 )      // Hz
  , dead_time_( 0.0 ) // ms
  , num_targets_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::poisson_generator_ps::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_;
  ( *d )[ names::dead_time ] = dead_time_;
}

void
nest::poisson_generator_ps::Parameters_::set( const DictionaryDatum& d, Node* node )
{

  updateValueParam< double >( d, names::dead_time, dead_time_, node );
  if ( dead_time_ < 0 )
  {
    throw BadProperty( "The dead time cannot be negative." );
  }

  updateValueParam< double >( d, names::rate, rate_, node );

  if ( rate_ < 0.0 )
  {
    throw BadProperty( "The rate cannot be negative." );
  }

  if ( 1000.0 / rate_ < dead_time_ )
  {
    throw BadProperty( "The inverse rate cannot be smaller than the dead time." );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::poisson_generator_ps::poisson_generator_ps()
  : DeviceNode()
  , device_()
  , P_()
{
}

nest::poisson_generator_ps::poisson_generator_ps( const poisson_generator_ps& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::poisson_generator_ps::init_state_( const Node& proto )
{
  const poisson_generator_ps& pr = downcast< poisson_generator_ps >( proto );

  device_.init_state( pr.device_ );
}

void
nest::poisson_generator_ps::init_buffers_()
{
  device_.init_buffers();

  // forget all about past, but do not discard connection information
  B_.next_spike_.clear();
  B_.next_spike_.resize( P_.num_targets_, Buffers_::SpikeTime( Time::neg_inf(), 0 ) );
}

void
nest::poisson_generator_ps::calibrate()
{
  device_.calibrate();
  if ( P_.rate_ > 0 )
  {
    V_.inv_rate_ms_ = 1000.0 / P_.rate_ - P_.dead_time_;
  }
  else
  {
    V_.inv_rate_ms_ = std::numeric_limits< double >::infinity();
  }

  /* The user may have set Device::start and/or origin to a later time
     during a simulation break. We can handle this in two ways:
     1. Generate intervals for the intervening period.
     2. Force re-initialization of the generator.
     I opt for variant 2, since it is more efficient. To be
     consistent across targets, all targets are reset even if a
     single one has a spike time before origin+start.
  */
  if ( not B_.next_spike_.empty() )
  {
    // find minimum time stamp, offset does not matter here
    Time min_time = B_.next_spike_.begin()->first;

    for ( std::vector< Buffers_::SpikeTime >::const_iterator it = B_.next_spike_.begin() + 1;
          it != B_.next_spike_.end();
          ++it )
    {
      min_time = std::min( min_time, it->first );
    }

    if ( min_time < device_.get_origin() + device_.get_start() )
    {
      B_.next_spike_.clear(); // will be resized with neg_infs below
    }
  }

  // If new targets have been added during a simulation break, we
  // initialize the new elements in next_spike_ with -1. The existing
  // elements are unchanged.
  if ( B_.next_spike_.size() == 0 )
  {
    B_.next_spike_.resize( P_.num_targets_, Buffers_::SpikeTime( Time::neg_inf(), 0 ) );
  }
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

void
nest::poisson_generator_ps::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  if ( P_.rate_ <= 0 || P_.num_targets_ == 0 )
  {
    return;
  }

  /* Limits of device activity.
   * The (excluded) lower boundary is the left edge of the slice, T + from.
   * The (included) upper boundary is the right edge of the slice, T + to.
   * of the slice.
   */
  V_.t_min_active_ = std::max( T + Time::step( from ), device_.get_origin() + device_.get_start() );
  V_.t_max_active_ = std::min( T + Time::step( to ), device_.get_origin() + device_.get_stop() );

  // Nothing to do for equality, since left boundary is excluded
  if ( V_.t_min_active_ < V_.t_max_active_ )
  {
    // we send the event as a "normal" event without offgrid-information
    // the event hook then sends out the real spikes with offgrid timing
    // We pretend to send at T+from
    DSSpikeEvent se;
    kernel().event_delivery_manager.send( *this, se, from );
  }
}

void
nest::poisson_generator_ps::event_hook( DSSpikeEvent& e )
{
  // get port number
  const port prt = e.get_port();

  // we handle only one port here, get reference to vector elem
  assert( 0 <= prt && static_cast< size_t >( prt ) < B_.next_spike_.size() );

  // obtain rng
  librandom::RngPtr rng = kernel().rng_manager.get_rng( get_thread() );

  // introduce nextspk as a shorthand
  Buffers_::SpikeTime& nextspk = B_.next_spike_[ prt ];

  if ( nextspk.first.is_neg_inf() )
  {
    // need to initialize relative to t_min_active_
    // first spike is drawn from backward recurrence time to initialize the
    // process in equilibrium.
    // In the case of the Poisson process with dead time, this has two domains:
    // one with uniform probability (t<dead_time) and one
    // with exponential probability (t>=dead_time).
    // First we draw a uniform number to choose the case according to the
    // associated probability mass.
    // If dead_time==0 we do not want to draw additional random numbers (keeps
    // old functionality).

    double spike_offset = 0;

    if ( P_.dead_time_ > 0 and rng->drand() < P_.dead_time_ * P_.rate_ / 1000.0 )
    {
      // uniform case: spike occurs with uniform probability in [0, dead_time].
      spike_offset = rng->drand() * P_.dead_time_;
    }
    else
    {
      // exponential case: spike occurs with exponential probability in
      // [dead_time, infinity]
      spike_offset = V_.inv_rate_ms_ * V_.exp_dev_( rng ) + P_.dead_time_;
    }

    // spike_offset is now time from t_min_active_ til first spike.
    // Split into stamp+offset, then add t_min_active.
    nextspk.first = Time::ms_stamp( spike_offset );
    nextspk.second = nextspk.first.get_ms() - spike_offset;
    nextspk.first += V_.t_min_active_;
  }

  // as long as there are spikes in active period, emit and redraw
  while ( nextspk.first <= V_.t_max_active_ )
  {
    // std::cerr << nextspk.first << '\t' << nextspk.second << '\n';
    e.set_stamp( nextspk.first );
    e.set_offset( nextspk.second );
    e.get_receiver().handle( e );

    // Draw time of next spike
    // Time of spike relative to current nextspk.first stamp
    const double new_offset = -nextspk.second + V_.inv_rate_ms_ * V_.exp_dev_( rng ) + P_.dead_time_;

    if ( new_offset < 0 ) // still in same stamp
    {
      nextspk.second = -new_offset; // stamps always 0 < stamp <= h
    }
    else
    {
      // split into stamp and offset, then add to old stamp
      const Time delta_stamp = Time::ms_stamp( new_offset );
      nextspk.first += delta_stamp;
      nextspk.second = delta_stamp.get_ms() - new_offset;
    }
  }
  // std::cerr << "********************************\n";
}
