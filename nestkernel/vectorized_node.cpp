/*
 *  vectorized_node.cpp
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

#include "vectorized_node.h"

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
VectorizedNode::VectorizedNode()
  : node_uses_wfr_( 0 )
  , frozen_( 0 )
  , initialized_( 0 )
  , global_ids( 0 )
  , thread( -1 )
{
}

void
VectorizedNode::reset()
{
  node_uses_wfr_.clear();
  frozen_.clear();
  initialized_.clear();
  global_ids.clear();
  thread = -1;
}
bool
VectorizedNode::wfr_update( const Time&, const long, const long, index )
{
  throw UnexpectedEvent( "Waveform relaxation not supported." );
}
void
VectorizedNode::set_frozen_( bool frozen, index local_id )
{
  frozen_.at( local_id ) = frozen;
}
port
VectorizedNode::send_test_event( Node&, rport, synindex, bool, index )
{
  throw IllegalConnection(
    "Source node does not send output.\n"
    "  Note that recorders must be connected as Connect(neuron, recorder)." );
}
void
VectorizedNode::register_stdp_connection( double, double, index )
{
  throw IllegalConnection( "The target node does not support STDP synapses." );
}
void
VectorizedNode::handle( SpikeEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle spike input." );
}
void
VectorizedNode::handle( WeightRecorderEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle weight recorder events." );
}
port
VectorizedNode::handles_test_event( WeightRecorderEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support weight recorder events." );
}

port
VectorizedNode::handles_test_event( SpikeEvent&, rport, index )
{
  throw IllegalConnection(
    "The target node or synapse model does not support spike input.\n"
    "  Note that volt/multimeters must be connected as Connect(meter, neuron)." );
}
void
VectorizedNode::handle( RateEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle rate input." );
}
port
VectorizedNode::handles_test_event( RateEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support rate input." );
}
void
VectorizedNode::handle( CurrentEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle current input." );
}
port
VectorizedNode::handles_test_event( CurrentEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support current input." );
}
void
VectorizedNode::handle( DataLoggingRequest&, index )
{
  throw UnexpectedEvent( "The target node does not handle data logging requests." );
}
port
VectorizedNode::handles_test_event( DataLoggingRequest&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support data logging requests." );
}
void
VectorizedNode::handle( DataLoggingReply&, index )
{
  throw UnexpectedEvent();
}

void
VectorizedNode::handle( ConductanceEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle conductance input." );
}

port
VectorizedNode::handles_test_event( ConductanceEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support conductance input." );
}

void
VectorizedNode::handle( DoubleDataEvent&, index )
{
  throw UnexpectedEvent();
}

port
VectorizedNode::handles_test_event( DoubleDataEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support double data event." );
}

port
VectorizedNode::handles_test_event( DSSpikeEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support spike input." );
}

port
VectorizedNode::handles_test_event( DSCurrentEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support DS current input." );
}

void
VectorizedNode::handle( GapJunctionEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle gap junction input." );
}

port
VectorizedNode::handles_test_event( GapJunctionEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support gap junction input." );
  return invalid_port_;
}

void
VectorizedNode::sends_secondary_event( GapJunctionEvent&, index )
{
  throw IllegalConnection( "The source node does not support gap junction output." );
}

void
VectorizedNode::handle( InstantaneousRateConnectionEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle instantaneous rate input." );
}

port
VectorizedNode::handles_test_event( InstantaneousRateConnectionEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support instantaneous rate input." );
  return invalid_port_;
}

void
VectorizedNode::sends_secondary_event( InstantaneousRateConnectionEvent&, index )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}

void
VectorizedNode::sends_secondary_event( DiffusionConnectionEvent&, index )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}


void
VectorizedNode::handle( DiffusionConnectionEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle diffusion input." );
}

port
VectorizedNode::handles_test_event( DiffusionConnectionEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support diffusion input." );
}


void
VectorizedNode::handle( DelayedRateConnectionEvent&, index )
{
  throw UnexpectedEvent( "The target node does not handle delayed rate input." );
}

port
VectorizedNode::handles_test_event( DelayedRateConnectionEvent&, rport, index )
{
  throw IllegalConnection( "The target node or synapse model does not support delayed rate input." );
}

void
VectorizedNode::sends_secondary_event( DelayedRateConnectionEvent&, index )
{
  throw IllegalConnection( "The source node does not support delayed rate output." );
}
double
VectorizedNode::get_K_value( double, index )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_LTD_value( double, index )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_K_values( double, double&, double&, double&, index )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_history( double,
  double,
  std::deque< histentry >::iterator*,
  std::deque< histentry >::iterator*,
  index )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_LTP_history( double,
  double,
  std::deque< histentry_extended >::iterator*,
  std::deque< histentry_extended >::iterator*,
  index )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_urbanczik_history( double,
  double,
  std::deque< histentry_extended >::iterator*,
  std::deque< histentry_extended >::iterator*,
  int,
  index )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_C_m( int, index )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_g_L( int, index )
{
  throw UnexpectedEvent();
}

double VectorizedNode::get_tau_Ca( index ) const
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_L( int, index )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_s( int, index )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_syn_ex( int, index )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_syn_in( int, index )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::event_hook( DSSpikeEvent& e, index )
{
  e.get_receiver().handle( e );
}
void
VectorizedNode::event_hook( DSCurrentEvent& e, index )
{
  e.get_receiver().handle( e );
}
index
VectorizedNode::get_global_id( index local_id ) const
{
  return global_ids.at( local_id );
}
void
VectorizedNode::insert_global_id( index id )
{
  global_ids.push_back( id );
}
index
VectorizedNode::size() const
{
  return global_ids.size();
}

void VectorizedNode::set_initialized_( index )
{
  // does nothing the base implementation
}

void
VectorizedNode::resize( index, index )
{
  // index current_size = global_ids.size();
  index total_space = global_ids.size();

  node_uses_wfr_.resize( total_space, false );
  frozen_.resize( total_space, false );
  initialized_.resize( total_space, false );
}

Node*
VectorizedNode::get_wrapper( index node_id, index ) const
{
  nest::index global_id = get_global_id( node_id );
  Node* node = kernel().node_manager.get_node_or_proxy( global_id, thread );
  return node;
}
}
