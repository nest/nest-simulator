/*
 *  step_current_generator.cpp
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

#include "step_current_generator.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{
RecordablesMap< step_current_generator >
  step_current_generator::recordablesMap_;

template <>
void
RecordablesMap< step_current_generator >::create()
{
  insert_( Name( names::I ), &step_current_generator::get_I_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::step_current_generator::Parameters_::Parameters_()
  : amp_times_()  // ms
  , amp_values_() // pA
{
}

nest::step_current_generator::Parameters_::Parameters_( const Parameters_& p )
  : amp_times_( p.amp_times_ )
  , amp_values_( p.amp_values_ )
{
}

nest::step_current_generator::Parameters_&
  nest::step_current_generator::Parameters_::
  operator=( const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  amp_times_ = p.amp_times_;
  amp_values_ = p.amp_values_;

  return *this;
}

nest::step_current_generator::State_::State_()
  : I_( 0.0 ) // pA
{
}

nest::step_current_generator::Buffers_::Buffers_( step_current_generator& n )
  : logger_( n )
{
}

nest::step_current_generator::Buffers_::Buffers_( const Buffers_&,
  step_current_generator& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::step_current_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ "amplitude_times" ] =
    DoubleVectorDatum( new std::vector< double >( amp_times_ ) );
  ( *d )[ "amplitude_values" ] =
    DoubleVectorDatum( new std::vector< double >( amp_values_ ) );
}

void
nest::step_current_generator::Parameters_::set( const DictionaryDatum& d,
  Buffers_& b )
{
  const bool ut =
    updateValue< std::vector< double > >( d, "amplitude_times", amp_times_ );
  const bool uv =
    updateValue< std::vector< double > >( d, "amplitude_values", amp_values_ );
  if ( ut xor uv )
  {
    throw BadProperty( "Amplitude times and values must be reset together." );
  }

  if ( amp_times_.size() != amp_values_.size() )
  {
    throw BadProperty( "Amplitude times and values have to be the same size." );
  }

  // ensure amp times are strictly monotonically increasing
  if ( not amp_times_.empty() )
  {
    std::vector< double >::const_iterator prev = amp_times_.begin();
    for ( std::vector< double >::const_iterator next = prev + 1;
          next != amp_times_.end();
          ++next, ++prev )
    {
      if ( *prev >= *next )
      {
        throw BadProperty( "Amplitude times must strictly increasing." );
      }
    }
  }
  if ( ut && uv )
  {
    b.idx_ = 0;
  } // reset if we got new data
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::step_current_generator::step_current_generator()
  : Node()
  , device_()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::step_current_generator::step_current_generator(
  const step_current_generator& n )
  : Node( n )
  , device_( n.device_ )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::step_current_generator::init_state_( const Node& proto )
{
  const step_current_generator& pr =
    downcast< step_current_generator >( proto );

  device_.init_state( pr.device_ );
}

void
nest::step_current_generator::init_buffers_()
{
  device_.init_buffers();
  B_.logger_.reset();

  B_.idx_ = 0;
  B_.amp_ = 0;
}

void
nest::step_current_generator::calibrate()
{
  B_.logger_.init();

  device_.calibrate();
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

void
nest::step_current_generator::update( Time const& origin,
  const long from,
  const long to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  assert( P_.amp_times_.size() == P_.amp_values_.size() );

  const long t0 = origin.get_steps();

  // Skip any times in the past. Since we must send events proactively,
  // idx_ must point to times in the future.
  const long first = t0 + from;
  while ( B_.idx_ < P_.amp_times_.size()
    && Time( Time::ms( P_.amp_times_[ B_.idx_ ] ) ).get_steps() <= first )
  {
    ++B_.idx_;
  }

  for ( long offs = from; offs < to; ++offs )
  {
    const long curr_time = t0 + offs;

    S_.I_ = 0.0;

    // Keep the amplitude up-to-date at all times.
    // We need to change the amplitude one step ahead of time, see comment
    // on class SimulatingDevice.
    if ( B_.idx_ < P_.amp_times_.size()
      && curr_time + 1
        == Time( Time::ms( P_.amp_times_[ B_.idx_ ] ) ).get_steps() )
    {
      B_.amp_ = P_.amp_values_[ B_.idx_ ];
      B_.idx_++;
    }

    // but send only if active
    if ( device_.is_active( Time::step( curr_time ) ) )
    {
      CurrentEvent ce;
      ce.set_current( B_.amp_ );
      S_.I_ = B_.amp_;
      kernel().event_delivery_manager.send( *this, ce, offs );
    }
    B_.logger_.record_data( origin.get_steps() + offs );
  }
}

void
nest::step_current_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
