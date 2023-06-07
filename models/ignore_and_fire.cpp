/*
 *  ignore_and_fire.cpp
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

#include "ignore_and_fire.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"
#include "propagator_stability.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "ring_buffer_impl.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

nest::RecordablesMap< nest::ignore_and_fire > nest::ignore_and_fire::recordablesMap_;

namespace nest
{

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< ignore_and_fire >::create()
{
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

ignore_and_fire::Parameters_::Parameters_()
  : phase_( 1.0 )
  , rate_( 10. )
{
}

ignore_and_fire::State_::State_()
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
ignore_and_fire::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::phase, phase_ );
  def< double >( d, names::rate, rate_ );
}

void
ignore_and_fire::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::phase, phase_, node );
  updateValueParam< double >( d, names::rate, rate_, node );

  if ( phase_ <= 0.0 or phase_> 1.0 )
  {
    throw BadProperty( "Phase must be between 0 and 1." );
  }
  
  if ( rate_ <= 0.0 )
  {
    throw BadProperty( "Firing rate must be > 0." );
  }
}

void
ignore_and_fire::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
}


void
ignore_and_fire::State_::set( const DictionaryDatum& d, const Parameters_& p, Node* node )
{
}


ignore_and_fire::Buffers_::Buffers_( ignore_and_fire& n )
  : logger_( n )
{
}

ignore_and_fire::Buffers_::Buffers_( const Buffers_&, ignore_and_fire& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

ignore_and_fire::ignore_and_fire()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  ignore_and_fire::calc_initial_variables_();
}

ignore_and_fire::ignore_and_fire( const ignore_and_fire& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  ignore_and_fire::calc_initial_variables_();
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
ignore_and_fire::init_buffers_()
{
  B_.input_buffer_.clear(); // includes resize

  B_.logger_.reset();

  ArchivingNode::clear_history();
}

void
ignore_and_fire::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();  
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
ignore_and_fire::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    // get read access to the correct input-buffer slot
    const index input_buffer_slot = kernel().event_delivery_manager.get_modulo( lag );
    auto& input = B_.input_buffer_.get_values_all_channels( input_buffer_slot );

    // threshold crossing
    if ( V_.phase_steps_ == 0 )
    {
      V_.phase_steps_ = V_.firing_period_steps_ - 1;
      
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }
    else
    { 
      --V_.phase_steps_;
    }    

    // reset all values in the currently processed input-buffer slot
    B_.input_buffer_.reset_values_all_channels( input_buffer_slot );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
ignore_and_fire::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  const index input_buffer_slot = kernel().event_delivery_manager.get_modulo(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) );
  const double s = e.get_weight() * e.get_multiplicity();

  // separate buffer channels for excitatory and inhibitory inputs
  B_.input_buffer_.add_value( input_buffer_slot, s > 0 ? Buffers_::SYN_EX : Buffers_::SYN_IN, s );
}

void
ignore_and_fire::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  
  const index input_buffer_slot = kernel().event_delivery_manager.get_modulo(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) );

  const double I = e.get_current();
  const double w = e.get_weight();

  B_.input_buffer_.add_value( input_buffer_slot, Buffers_::I0, w * I );
}

void
ignore_and_fire::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
