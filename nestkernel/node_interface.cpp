/*
 *  node_interface.cpp
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


#include "node_interface.h"

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "exceptions.h"


namespace nest
{

NodeInterface::NodeInterface()
{
}

NodeInterface::NodeInterface( const NodeInterface& n )
{
}

NodeInterface::~NodeInterface()
{
}


DictionaryDatum
NodeInterface::get_status_dict_()
{
  return DictionaryDatum( new Dictionary );
}

void
NodeInterface::set_local_device_id( const index )
{
  assert( false and "set_local_device_id() called on a non-device node of type" );
}

index
NodeInterface::get_local_device_id() const
{
  assert( false and "get_local_device_id() called on a non-device node." );
  return invalid_index;
}


/**
 * Default implementation of wfr_update just
 * throws UnexpectedEvent
 */
bool
NodeInterface::wfr_update( Time const&, const long, const long )
{
  throw UnexpectedEvent( "Waveform relaxation not supported." );
}

/**
 * Default implementation of check_connection just throws IllegalConnection
 */
port
NodeInterface::send_test_event( NodeInterface&, rport, synindex, bool )
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
NodeInterface::register_stdp_connection( double, double )
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
NodeInterface::handle( SpikeEvent& )
{
  throw UnexpectedEvent( "The target node does not handle spike input." );
}

port
NodeInterface::handles_test_event( SpikeEvent&, rport )
{
  throw IllegalConnection(
    "The target node or synapse model does not support spike input.\n"
    "  Note that volt/multimeters must be connected as Connect(meter, neuron)." );
}

void
NodeInterface::handle( WeightRecorderEvent& )
{
  throw UnexpectedEvent( "The target node does not handle weight recorder events." );
}

port
NodeInterface::handles_test_event( WeightRecorderEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support weight recorder events." );
}

void
NodeInterface::handle( RateEvent& )
{
  throw UnexpectedEvent( "The target node does not handle rate input." );
}

port
NodeInterface::handles_test_event( RateEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support rate input." );
}

void
NodeInterface::handle( CurrentEvent& )
{
  throw UnexpectedEvent( "The target node does not handle current input." );
}

port
NodeInterface::handles_test_event( CurrentEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support current input." );
}

void
NodeInterface::handle( DataLoggingRequest& )
{
  throw UnexpectedEvent( "The target node does not handle data logging requests." );
}

port
NodeInterface::handles_test_event( DataLoggingRequest&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support data logging requests." );
}

void
NodeInterface::handle( DataLoggingReply& )
{
  throw UnexpectedEvent();
}

void
NodeInterface::handle( ConductanceEvent& )
{
  throw UnexpectedEvent( "The target node does not handle conductance input." );
}

port
NodeInterface::handles_test_event( ConductanceEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support conductance input." );
}

void
NodeInterface::handle( DoubleDataEvent& )
{
  throw UnexpectedEvent();
}

port
NodeInterface::handles_test_event( DoubleDataEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support double data event." );
}

port
NodeInterface::handles_test_event( DSSpikeEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support spike input." );
}

port
NodeInterface::handles_test_event( DSCurrentEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support DS current input." );
}

void
NodeInterface::handle( GapJunctionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle gap junction input." );
}

port
NodeInterface::handles_test_event( GapJunctionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support gap junction input." );
}

void
NodeInterface::sends_secondary_event( GapJunctionEvent& )
{
  throw IllegalConnection( "The source node does not support gap junction output." );
}

void
NodeInterface::handle( InstantaneousRateConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle instantaneous rate input." );
}

port
NodeInterface::handles_test_event( InstantaneousRateConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support instantaneous rate input." );
}

void
NodeInterface::sends_secondary_event( InstantaneousRateConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}

void
NodeInterface::handle( DiffusionConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle diffusion input." );
}

port
NodeInterface::handles_test_event( DiffusionConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support diffusion input." );
}

void
NodeInterface::sends_secondary_event( DiffusionConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support diffusion output." );
}

void
NodeInterface::handle( DelayedRateConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle delayed rate input." );
}

port
NodeInterface::handles_test_event( DelayedRateConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support delayed rate input." );
}

void
NodeInterface::sends_secondary_event( DelayedRateConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support delayed rate output." );
}


double
NodeInterface::get_LTD_value( double )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_K_value( double )
{
  throw UnexpectedEvent();
}


void
NodeInterface::get_K_values( double, double&, double&, double& )
{
  throw UnexpectedEvent();
}

void
NodeInterface::get_history( double, double, std::deque< histentry >::iterator*, std::deque< histentry >::iterator* )
{
  throw UnexpectedEvent();
}

void
NodeInterface::get_LTP_history( double,
  double,
  std::deque< histentry_extended >::iterator*,
  std::deque< histentry_extended >::iterator* )
{
  throw UnexpectedEvent();
}

void
NodeInterface::get_urbanczik_history( double,
  double,
  std::deque< histentry_extended >::iterator*,
  std::deque< histentry_extended >::iterator*,
  int )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_C_m( int )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_g_L( int )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_tau_L( int )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_tau_s( int )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_tau_syn_ex( int )
{
  throw UnexpectedEvent();
}

double
NodeInterface::get_tau_syn_in( int )
{
  throw UnexpectedEvent();
}

void
NodeInterface::event_hook( DSSpikeEvent& e )
{
}

void
NodeInterface::event_hook( DSCurrentEvent& e )
{
}

} // namespace
