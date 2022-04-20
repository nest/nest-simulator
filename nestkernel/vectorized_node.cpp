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
VectorizedNode::wfr_update( const Time&, const long, const long, index local_id )
{
  throw UnexpectedEvent( "Waveform relaxation not supported." );
}
void
VectorizedNode::set_frozen_( bool frozen, index local_id )
{
  frozen_.at( local_id ) = frozen;
}
port
VectorizedNode::send_test_event( Node& receiving_node,
  rport receptor_type,
  synindex syn_id,
  bool dummy_target,
  index local_id )
{
  throw IllegalConnection(
    "Source node does not send output.\n"
    "  Note that recorders must be connected as Connect(neuron, recorder)." );
}
void
VectorizedNode::register_stdp_connection( double, double, index local_id )
{
  throw IllegalConnection( "The target node does not support STDP synapses." );
}
void
VectorizedNode::handle( SpikeEvent& e, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle spike input." );
}
void
VectorizedNode::handle( WeightRecorderEvent&, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle weight recorder events." );
}
port
VectorizedNode::handles_test_event( WeightRecorderEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support weight recorder events." );
}

port
VectorizedNode::handles_test_event( SpikeEvent&, rport receptor_type, index local_id )
{
  throw IllegalConnection(
    "The target node or synapse model does not support spike input.\n"
    "  Note that volt/multimeters must be connected as Connect(meter, neuron)." );
}
void
VectorizedNode::handle( RateEvent& e, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle rate input." );
}
port
VectorizedNode::handles_test_event( RateEvent&, rport receptor_type, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support rate input." );
}
void
VectorizedNode::handle( CurrentEvent& e, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle current input." );
}
port
VectorizedNode::handles_test_event( CurrentEvent&, rport receptor_type, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support current input." );
}
void
VectorizedNode::handle( DataLoggingRequest& e, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle data logging requests." );
}
port
VectorizedNode::handles_test_event( DataLoggingRequest&, rport receptor_type, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support data logging requests." );
}
void
VectorizedNode::handle( DataLoggingReply&, index local_id )
{
  throw UnexpectedEvent();
}

void
VectorizedNode::handle( ConductanceEvent&, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle conductance input." );
}

port
VectorizedNode::handles_test_event( ConductanceEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support conductance input." );
}

void
VectorizedNode::handle( DoubleDataEvent&, index local_id )
{
  throw UnexpectedEvent();
}

port
VectorizedNode::handles_test_event( DoubleDataEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support double data event." );
}

port
VectorizedNode::handles_test_event( DSSpikeEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support spike input." );
}

port
VectorizedNode::handles_test_event( DSCurrentEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support DS current input." );
}

void
VectorizedNode::handle( GapJunctionEvent&, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle gap junction input." );
}

port
VectorizedNode::handles_test_event( GapJunctionEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support gap junction input." );
  return invalid_port_;
}

void
VectorizedNode::sends_secondary_event( GapJunctionEvent&, index local_id )
{
  throw IllegalConnection( "The source node does not support gap junction output." );
}

void
VectorizedNode::handle( InstantaneousRateConnectionEvent&, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle instantaneous rate input." );
}

port
VectorizedNode::handles_test_event( InstantaneousRateConnectionEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support instantaneous rate input." );
  return invalid_port_;
}

void
VectorizedNode::sends_secondary_event( InstantaneousRateConnectionEvent&, index local_id )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}

void
VectorizedNode::sends_secondary_event( DiffusionConnectionEvent& de, index local_id )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}


void
VectorizedNode::handle( DiffusionConnectionEvent&, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle diffusion input." );
}

port
VectorizedNode::handles_test_event( DiffusionConnectionEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support diffusion input." );
}


void
VectorizedNode::handle( DelayedRateConnectionEvent&, index local_id )
{
  throw UnexpectedEvent( "The target node does not handle delayed rate input." );
}

port
VectorizedNode::handles_test_event( DelayedRateConnectionEvent&, rport, index local_id )
{
  throw IllegalConnection( "The target node or synapse model does not support delayed rate input." );
}

void
VectorizedNode::sends_secondary_event( DelayedRateConnectionEvent&, index local_id )
{
  throw IllegalConnection( "The source node does not support delayed rate output." );
}
double
VectorizedNode::get_K_value( double t, index local_id )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_LTD_value( double t, index local_id )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_K_values( double t,
  double& Kminus,
  double& nearest_neighbor_Kminus,
  double& Kminus_triplet,
  index local_id )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_history( double t1,
  double t2,
  std::deque< histentry >::iterator* start,
  std::deque< histentry >::iterator* finish,
  index local_id )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_LTP_history( double t1,
  double t2,
  std::deque< histentry_extended >::iterator* start,
  std::deque< histentry_extended >::iterator* finish,
  index local_id )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::get_urbanczik_history( double t1,
  double t2,
  std::deque< histentry_extended >::iterator* start,
  std::deque< histentry_extended >::iterator* finish,
  int value,
  index local_id )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_C_m( int comp, index local_id )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_g_L( int comp, index local_id )
{
  throw UnexpectedEvent();
}

double
VectorizedNode::get_tau_Ca( index local_id ) const
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_L( int comp, index local_id )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_s( int comp, index local_id )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_syn_ex( int comp, index local_id )
{
  throw UnexpectedEvent();
}
double
VectorizedNode::get_tau_syn_in( int comp, index local_id )
{
  throw UnexpectedEvent();
}
void
VectorizedNode::event_hook( DSSpikeEvent& e, index local_id )
{
  e.get_receiver().handle( e );
}
void
VectorizedNode::event_hook( DSCurrentEvent& e, index local_id )
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

void
VectorizedNode::set_initialized_( index local_id )
{
  // does nothing the base implementation
}

void
VectorizedNode::resize( index extended_space, index thread_id )
{
  // index current_size = global_ids.size();
  index total_space = global_ids.size();

  node_uses_wfr_.resize( total_space, false );
  frozen_.resize( total_space, false );
  initialized_.resize( total_space, false );
}

Node*
VectorizedNode::get_wrapper( index node_id, index thread_id ) const
{
  assert( node_id >= 0 );
  nest::index global_id = get_global_id( node_id );
  Node* node = kernel().node_manager.get_node_or_proxy( global_id, thread );
  return node;
}
}