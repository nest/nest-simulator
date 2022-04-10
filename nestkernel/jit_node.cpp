/*
 *  jit_node.cpp
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

#include "jit_node.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"


// Includes from libnestutil:
#include "compose.hpp"
#include "event.h"

namespace nest
{

JitNode::JitNode()
  : local_id_( -1 )
  , pos_in_thread( -1 )
{
}

JitNode::~JitNode() noexcept
{
  container_->reset();
}

JitNode::JitNode( const JitNode& n )
  : Node( n )
  , local_id_( -1 )
  , pos_in_thread( -1 )
  , container_( n.container_ )
{
}
void
JitNode::resize( index extended_space )
{
  container_->resize( extended_space );
}
void
JitNode::reset_node()
{
  local_id_ = -1;
  pos_in_thread = -1;
  container_->reset();
}
bool
JitNode::supports_urbanczik_archiving() const
{
  return false;
}
bool
JitNode::local_receiver() const
{
  return false;
}
bool
JitNode::one_node_per_process() const
{
  return false;
}
bool
JitNode::is_off_grid() const
{
  return false;
}
bool
JitNode::is_proxy() const
{
  return false;
}

index
JitNode::get_node_id() const
{
  return container_->get_global_id( local_id_ );
}


bool
JitNode::is_frozen() const
{
  return container_->is_frozen( local_id_ );
}
bool
JitNode::node_uses_wfr() const
{
  return container_->node_uses_wfr( local_id_ );
}
void
JitNode::set_node_uses_wfr( const bool value )
{
  container_->set_node_uses_wfr( value, local_id_ );
}
void
JitNode::init()
{
  container_->init( local_id_ );
}
void
JitNode::calibrate()
{
  container_->calibrate( local_id_ );
}

void
JitNode::calibrate_time( const TimeConverter& time_converter )
{
  container_->calibrate_time( time_converter, local_id_ );
}
void
JitNode::post_run_cleanup()
{
  container_->post_run_cleanup( local_id_ );
}
void
JitNode::finalize()
{
  container_->finalize( local_id_ );
}
void
JitNode::update( const Time& network_time, const long initial_step, const long post_final )
{
  container_->update( network_time, initial_step, post_final, local_id_ );
}
bool
JitNode::wfr_update( const Time& network_time, const long initial_step, const long post_final )
{
  return container_->wfr_update( network_time, initial_step, post_final, local_id_ );
}
void
JitNode::set_status( const DictionaryDatum& d )
{
  container_->set_status( d, local_id_ );
}
void
JitNode::get_status( DictionaryDatum& d ) const
{
  ( *d )[ Name( "is_vectorized" ) ] = true;
  container_->get_status( d, local_id_ );
}
port
JitNode::send_test_event( Node& receiving_node, rport receptor_type, synindex syn_id, bool dummy_target )
{
  return container_->send_test_event( receiving_node, receptor_type, syn_id, dummy_target, local_id_ );
}

port
JitNode::handles_test_event( SpikeEvent& spike, rport receptor_type )
{
  return container_->handles_test_event( spike, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( WeightRecorderEvent& weightRecorder, rport receptor_type )
{
  return container_->handles_test_event( weightRecorder, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( RateEvent& rate, rport receptor_type )
{
  return container_->handles_test_event( rate, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( DataLoggingRequest& dataLogging_request, rport receptor_type )
{
  return container_->handles_test_event( dataLogging_request, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( CurrentEvent& current, rport receptor_type )
{
  return container_->handles_test_event( current, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( ConductanceEvent& conductance, rport receptor_type )
{
  return container_->handles_test_event( conductance, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( DoubleDataEvent& double_data, rport receptor_type )
{
  return container_->handles_test_event( double_data, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( DSCurrentEvent& ds_current, rport receptor_type )
{
  return container_->handles_test_event( ds_current, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( DSSpikeEvent& ds_spike, rport receptor_type )
{
  return container_->handles_test_event( ds_spike, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( GapJunctionEvent& gap_junction, rport receptor_type )
{
  return container_->handles_test_event( gap_junction, receptor_type, local_id_ );
}

port
JitNode::handles_test_event( InstantaneousRateConnectionEvent& instantaneous_rate_connection, rport receptor_type )
{
  return container_->handles_test_event( instantaneous_rate_connection, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( DiffusionConnectionEvent& diffusion_connection, rport receptor_type )
{
  return container_->handles_test_event( diffusion_connection, receptor_type, local_id_ );
}
port
JitNode::handles_test_event( DelayedRateConnectionEvent& delayed_rate_connection, rport receptor_type )
{
  return container_->handles_test_event( delayed_rate_connection, receptor_type, local_id_ );
}
void
JitNode::sends_secondary_event( GapJunctionEvent& ge )
{
  container_->sends_secondary_event( ge, local_id_ );
}
void
JitNode::sends_secondary_event( InstantaneousRateConnectionEvent& re )
{
  container_->sends_secondary_event( re, local_id_ );
}
void
JitNode::sends_secondary_event( DelayedRateConnectionEvent& re )
{
  container_->sends_secondary_event( re, local_id_ );
}

void
JitNode::sends_secondary_event( DiffusionConnectionEvent& de )
{
  container_->sends_secondary_event( de, local_id_ );
}
void
JitNode::register_stdp_connection( double a, double b )
{
  container_->register_stdp_connection( a, b, local_id_ );
}

void
JitNode::handle( RateEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( SpikeEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( CurrentEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( DoubleDataEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( ConductanceEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( DataLoggingReply& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( GapJunctionEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( DataLoggingRequest& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( WeightRecorderEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( DiffusionConnectionEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( DelayedRateConnectionEvent& e )
{
  container_->handle( e, local_id_ );
}
void
JitNode::handle( InstantaneousRateConnectionEvent& e )
{
  container_->handle( e, local_id_ );
}

double
JitNode::get_Ca_minus() const
{
  return container_->get_Ca_minus( local_id_ );
}
double
JitNode::get_synaptic_elements( Name name ) const
{
  return container_->get_synaptic_elements( name, local_id_ );
}

int
JitNode::get_synaptic_elements_vacant( Name name ) const
{
  return container_->get_synaptic_elements_vacant( name, local_id_ );
}
int
JitNode::get_synaptic_elements_connected( Name name ) const
{
  return container_->get_synaptic_elements_connected( name, local_id_ );
}
std::map< Name, double >
JitNode::get_synaptic_elements() const
{
  return container_->get_synaptic_elements( local_id_ );
}
void
JitNode::update_synaptic_elements( double value )
{
  container_->update_synaptic_elements( value, local_id_ );
}
void
JitNode::decay_synaptic_elements_vacant()
{
  container_->decay_synaptic_elements_vacant( local_id_ );
}
void
JitNode::connect_synaptic_element( Name name, int number )
{
  container_->connect_synaptic_element( name, number, local_id_ );
}
double
JitNode::get_K_value( double t )
{
  return container_->get_K_value( t, local_id_ );
}
double
JitNode::get_LTD_value( double t )
{
  return container_->get_LTD_value( t, local_id_ );
}
void
JitNode::get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet )
{
  container_->get_K_values( t, Kminus, nearest_neighbor_Kminus, Kminus_triplet, local_id_ );
}
void
JitNode::get_history( double t1,
  double t2,
  std::deque< histentry >::iterator* start,
  std::deque< histentry >::iterator* finish )
{
  container_->get_history( t1, t2, start, finish, local_id_ );
}
void
JitNode::get_LTP_history( double t1,
  double t2,
  std::deque< histentry_extended >::iterator* start,
  std::deque< histentry_extended >::iterator* finish )
{
  container_->get_LTP_history( t1, t2, start, finish, local_id_ );
}

void
JitNode::get_urbanczik_history( double t1,
  double t2,
  std::deque< histentry_extended >::iterator* start,
  std::deque< histentry_extended >::iterator* finish,
  int value )
{
  container_->get_urbanczik_history( t1, t2, start, finish, value, local_id_ );
}
double
JitNode::get_C_m( int comp )
{
  return container_->get_C_m( comp, local_id_ );
}
double
JitNode::get_g_L( int comp )
{
  return container_->get_g_L( comp, local_id_ );
}
double
JitNode::get_tau_L( int comp )
{
  return container_->get_tau_L( comp, local_id_ );
}
double
JitNode::get_tau_s( int comp )
{
  return container_->get_tau_s( comp, local_id_ );
}
double
JitNode::get_tau_syn_ex( int comp )
{
  return container_->get_tau_syn_ex( comp, local_id_ );
}
double
JitNode::get_tau_syn_in( int comp )
{
  return container_->get_tau_syn_in( comp, local_id_ );
}
void
JitNode::event_hook( DSSpikeEvent& ds_spike )
{
  container_->event_hook( ds_spike, local_id_ );
}
void
JitNode::event_hook( DSCurrentEvent& ds_current )
{
  container_->event_hook( ds_current, local_id_ );
}
void
JitNode::set_initialized_()
{
  container_->set_initialized_( local_id_ );
}
void
JitNode::set_frozen_( bool frozen )
{
  container_->set_frozen_( frozen, local_id_ );
}


void
JitNode::set_node_id_( index id )
{
  container_->insert_global_id( id );
  local_id_ = container_->size() - 1;
}

void
JitNode::set_pos_in_thread( index pos )
{
  pos_in_thread = pos;
}

index
JitNode::get_pos_in_thread() const
{
  return pos_in_thread;
}
void
JitNode::set_container( std::shared_ptr< VectorizedNode > container )
{
  container_ = container;
}

SignalType
JitNode::sends_signal() const
{
  return container_->sends_signal( local_id_ );
}
SignalType
JitNode::receives_signal() const
{
  return container_->receives_signal( local_id_ );
}
void
JitNode::set_status_base( const DictionaryDatum& dict )
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
  bool value = container_->is_frozen( local_id_ );
  updateValue< bool >( dict, names::frozen, value );
  container_->set_frozen_( value, local_id_ );
}
void
JitNode::init_state_()
{
  container_->init_state_( local_id_ );
}

void
JitNode::init_buffers_()
{
  container_->init_buffers_( local_id_ );
}
}
