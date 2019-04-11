/*
 *  sli_neuron.cpp
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

#include "sli_neuron.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "compose.hpp"
#include "numerics.h"

// includes from nest:
#include "neststartup.h" // get_engine()

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictstack.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::sli_neuron > nest::sli_neuron::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< sli_neuron >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &sli_neuron::get_V_m_ );
}
}

nest::sli_neuron::Buffers_::Buffers_( sli_neuron& n )
  : logger_( n )
{
}

nest::sli_neuron::Buffers_::Buffers_( const Buffers_&, sli_neuron& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::sli_neuron::sli_neuron()
  : Archiving_Node()
  , state_( new Dictionary() )
  , B_( *this )
{
  // We add empty defaults for /calibrate and /update, so that the uninitialized
  // node runs without errors.
  state_->insert( names::calibrate, new ProcedureDatum() );
  state_->insert( names::update, new ProcedureDatum() );
  recordablesMap_.create();
}

nest::sli_neuron::sli_neuron( const sli_neuron& n )
  : Archiving_Node( n )
  , state_( new Dictionary( *n.state_ ) )
  , B_( n.B_, *this )
{
  init_state_( n );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::sli_neuron::init_state_( const Node& proto )
{
  const sli_neuron& pr = downcast< sli_neuron >( proto );
  state_ = DictionaryDatum( new Dictionary( *pr.state_ ) );
}

void
nest::sli_neuron::init_buffers_()
{
  B_.ex_spikes_.clear(); // includes resize
  B_.in_spikes_.clear(); // includes resize
  B_.currents_.clear();  // includes resize
  B_.logger_.reset();    // includes resize
  Archiving_Node::clear_history();
}


void
nest::sli_neuron::calibrate()
{
  B_.logger_.init();

  if ( not state_->known( names::calibrate ) )
  {
    std::string msg = String::compose( "Node %1 has no /calibrate function in its status dictionary.", get_gid() );
    throw BadProperty( msg );
  }

  if ( not state_->known( names::update ) )
  {
    std::string msg = String::compose( "Node %1 has no /update function in its status dictionary", get_gid() );
    throw BadProperty( msg );
  }

#pragma omp critical( sli_neuron )
  {
    execute_sli_protected( state_, names::calibrate_node ); // call interpreter
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::sli_neuron::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );
  ( *state_ )[ names::t_origin ] = origin.get_steps();

  if ( state_->known( names::error ) )
  {
    std::string msg = String::compose( "Node %1 still has its error state set.", get_gid() );
    throw KernelException( msg );
  }

  for ( long lag = from; lag < to; ++lag )
  {
    ( *state_ )[ names::in_spikes ] = B_.in_spikes_.get_value( lag ); // in spikes arriving at right border
    ( *state_ )[ names::ex_spikes ] = B_.ex_spikes_.get_value( lag ); // ex spikes arriving at right border
    ( *state_ )[ names::currents ] = B_.currents_.get_value( lag );
    ( *state_ )[ names::t_lag ] = lag;

#pragma omp critical( sli_neuron )
    {
      execute_sli_protected( state_, names::update_node ); // call interpreter
    }

    bool spike_emission = false;
    if ( state_->known( names::spike ) )
    {
      spike_emission = ( *state_ )[ names::spike ];
    }

    // threshold crossing
    if ( spike_emission )
    {
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

/**
 * This function is not thread save and has to be called inside a omp critical
 * region.
 */
int
nest::sli_neuron::execute_sli_protected( DictionaryDatum state, Name cmd )
{
  SLIInterpreter& i = get_engine();

  i.DStack->push( state ); // push state dictionary as top namespace
  size_t exitlevel = i.EStack.load();
  i.EStack.push( new NameDatum( cmd ) );
  int result = i.execute_( exitlevel );
  i.DStack->pop(); // pop neuron's namespace

  if ( state->known( "error" ) )
  {
    assert( state->known( names::global_id ) );
    index g_id = ( *state )[ names::global_id ];
    std::string model = getValue< std::string >( ( *state )[ names::model ] );
    std::string msg = String::compose( "Error in %1 with global id %2.", model, g_id );
    throw KernelException( msg );
  }

  return result;
}

void
nest::sli_neuron::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.ex_spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.in_spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::sli_neuron::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double I = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * I );
}

void
nest::sli_neuron::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
