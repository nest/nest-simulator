/*
 *  node.cpp
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


#include "node.h"

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"
#include "namedatum.h"

namespace nest
{

Node::Node()
  : deprecation_warning()
  , node_id_( 0 )
  , thread_lid_( invalid_index )
  , model_id_( -1 )
  , thread_( 0 )
  , vp_( invalid_thread )
  , frozen_( false )
  , initialized_( false )
  , node_uses_wfr_( false )
{
}

Node::Node( const Node& n )
  : deprecation_warning( n.deprecation_warning )
  , node_id_( 0 )
  , thread_lid_( n.thread_lid_ )
  , model_id_( n.model_id_ )
  , thread_( n.thread_ )
  , vp_( n.vp_ )
  , frozen_( n.frozen_ )
  // copy must always initialized its own buffers
  , initialized_( false )
  , node_uses_wfr_( n.node_uses_wfr_ )
{
}

Node::~Node()
{
}

void
Node::init_state_()
{
}

void
Node::init()
{
  if ( initialized_ )
  {
    return;
  }

  init_state_();
  init_buffers_();

  initialized_ = true;
}

void
Node::init_buffers_()
{
}

void
Node::set_initialized()
{
  set_initialized_();
}

void
Node::set_initialized_()
{
}

std::string
Node::get_name() const
{
  if ( model_id_ < 0 )
  {
    return std::string( "UnknownNode" );
  }

  return kernel().model_manager.get_node_model( model_id_ )->get_name();
}

Model&
Node::get_model_() const
{
  assert( model_id_ >= 0 );
  return *kernel().model_manager.get_node_model( model_id_ );
}

DictionaryDatum
Node::get_status_dict_()
{
  return DictionaryDatum( new Dictionary );
}

void
Node::set_local_device_id( const index )
{
  assert( false and "set_local_device_id() called on a non-device node of type" );
}

index
Node::get_local_device_id() const
{
  assert( false and "get_local_device_id() called on a non-device node." );
  return invalid_index;
}

DictionaryDatum
Node::get_status_base()
{
  DictionaryDatum dict = get_status_dict_();

  // add information available for all nodes
  ( *dict )[ names::local ] = kernel().node_manager.is_local_node( this );
  ( *dict )[ names::model ] = LiteralDatum( get_name() );
  ( *dict )[ names::model_id ] = get_model_id();
  ( *dict )[ names::global_id ] = get_node_id();
  ( *dict )[ names::vp ] = get_vp();
  ( *dict )[ names::element_type ] = LiteralDatum( get_element_type() );

  // add information available only for local nodes
  if ( not is_proxy() )
  {
    ( *dict )[ names::frozen ] = is_frozen();
    ( *dict )[ names::node_uses_wfr ] = node_uses_wfr();
    ( *dict )[ names::thread_local_id ] = get_thread_lid();
    ( *dict )[ names::thread ] = get_thread();
  }

  // now call the child class' hook
  get_status( dict );

  return dict;
}

void
Node::set_status_base( const DictionaryDatum& dict )
{
  try
  {
    set_status( dict );
  }
  catch ( BadProperty& e )
  {
    throw BadProperty(
      String::compose( "Setting status of a '%1' with node ID %2: %3", get_name(), get_node_id(), e.message() ) );
  }

  updateValue< bool >( dict, names::frozen, frozen_ );
}

/**
 * Default implementation of wfr_update just
 * throws UnexpectedEvent
 */
bool
Node::wfr_update( Time const&, const long, const long )
{
  throw UnexpectedEvent( "Waveform relaxation not supported." );
}

/**
 * Default implementation of check_connection just throws IllegalConnection
 */
port
Node::send_test_event( Node&, rport, synindex, bool )
{
  throw IllegalConnection(
    "Source node does not send output.\n"
    "  Note that recorders must be connected as Connect(neuron, recorder)." );
}

/**
 * Default implementation of register_stdp_connection() just
 * throws IllegalConnection
 */
void
Node::register_stdp_connection( double, double )
{
  throw IllegalConnection( "The target node does not support STDP synapses." );
}

/**
 * Default implementation of event handlers just throws
 * an UnexpectedEvent exception.
 * @see class UnexpectedEvent
 * @throws UnexpectedEvent  This is the default event to throw.
 */
void
Node::handle( SpikeEvent& )
{
  throw UnexpectedEvent( "The target node does not handle spike input." );
}

port
Node::handles_test_event( SpikeEvent&, rport )
{
  throw IllegalConnection(
    "The target node or synapse model does not support spike input.\n"
    "  Note that volt/multimeters must be connected as Connect(meter, neuron)." );
}

void
Node::handle( WeightRecorderEvent& )
{
  throw UnexpectedEvent( "The target node does not handle weight recorder events." );
}

port
Node::handles_test_event( WeightRecorderEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support weight recorder events." );
}

void
Node::handle( RateEvent& )
{
  throw UnexpectedEvent( "The target node does not handle rate input." );
}

port
Node::handles_test_event( RateEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support rate input." );
}

void
Node::handle( CurrentEvent& )
{
  throw UnexpectedEvent( "The target node does not handle current input." );
}

port
Node::handles_test_event( CurrentEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support current input." );
}

void
Node::handle( DataLoggingRequest& )
{
  throw UnexpectedEvent( "The target node does not handle data logging requests." );
}

port
Node::handles_test_event( DataLoggingRequest&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support data logging requests." );
}

void
Node::handle( DataLoggingReply& )
{
  throw UnexpectedEvent();
}

void
Node::handle( ConductanceEvent& )
{
  throw UnexpectedEvent( "The target node does not handle conductance input." );
}

port
Node::handles_test_event( ConductanceEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support conductance input." );
}

void
Node::handle( DoubleDataEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( DoubleDataEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support double data event." );
}

port
Node::handles_test_event( DSSpikeEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support spike input." );
}

port
Node::handles_test_event( DSCurrentEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support DS current input." );
}

void
Node::handle( GapJunctionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle gap junction input." );
}

port
Node::handles_test_event( GapJunctionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support gap junction input." );
}

void
Node::sends_secondary_event( GapJunctionEvent& )
{
  throw IllegalConnection( "The source node does not support gap junction output." );
}

void
Node::handle( InstantaneousRateConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle instantaneous rate input." );
}

port
Node::handles_test_event( InstantaneousRateConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support instantaneous rate input." );
}

void
Node::sends_secondary_event( InstantaneousRateConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}

void
Node::handle( DiffusionConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle diffusion input." );
}

port
Node::handles_test_event( DiffusionConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support diffusion input." );
}

void
Node::sends_secondary_event( DiffusionConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support diffusion output." );
}

void
Node::handle( DelayedRateConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle delayed rate input." );
}

port
Node::handles_test_event( DelayedRateConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support delayed rate input." );
}

void
Node::sends_secondary_event( DelayedRateConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support delayed rate output." );
}


double
Node::get_LTD_value( double )
{
  throw UnexpectedEvent();
}

double
Node::get_K_value( double )
{
  throw UnexpectedEvent();
}


void
Node::get_K_values( double, double&, double&, double& )
{
  throw UnexpectedEvent();
}

void
nest::Node::get_history( double, double, std::deque< histentry >::iterator*, std::deque< histentry >::iterator* )
{
  throw UnexpectedEvent();
}

void
nest::Node::get_LTP_history( double,
  double,
  std::deque< histentry_extended >::iterator*,
  std::deque< histentry_extended >::iterator* )
{
  throw UnexpectedEvent();
}

void
nest::Node::get_urbanczik_history( double,
  double,
  std::deque< histentry_extended >::iterator*,
  std::deque< histentry_extended >::iterator*,
  int )
{
  throw UnexpectedEvent();
}

double
nest::Node::get_C_m( int )
{
  throw UnexpectedEvent();
}

double
nest::Node::get_g_L( int )
{
  throw UnexpectedEvent();
}

double
nest::Node::get_tau_L( int )
{
  throw UnexpectedEvent();
}

double
nest::Node::get_tau_s( int )
{
  throw UnexpectedEvent();
}

double
nest::Node::get_tau_syn_ex( int )
{
  throw UnexpectedEvent();
}

double
nest::Node::get_tau_syn_in( int )
{
  throw UnexpectedEvent();
}

void
Node::event_hook( DSSpikeEvent& e )
{
  e.get_receiver().handle( e );
}

void
Node::event_hook( DSCurrentEvent& e )
{
  e.get_receiver().handle( e );
}

} // namespace
