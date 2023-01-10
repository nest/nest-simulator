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
#include "logging.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"
#include "namedatum.h"

namespace nest
{

Node::Node()
  : CommonInterface()
{
  populate_data();
}

Node::Node( const Node& n )
  : CommonInterface( n )
{
  std::map< std::string, boost::any > n_base_data = n.data.at( 0 );

  n_base_data[ "node_id_" ] = size_t( 0 );
  // copy must always initialized its own buffers
  n_base_data[ "initialized_" ] = false;
  n_base_data[ "nc_ptr_" ] = nullptr;

  data.push_back( n_base_data );
}

Node::Node( std::map< std::string, boost::any > base_data )
  : CommonInterface()
{
  data.push_back( base_data );
}

Node::~Node()
{
}

void
Node::populate_data()
{

  std::map< std::string, boost::any > base_data;

  index zero_index_instance = 0;
  thread zero_thread_instance = 0;

  base_data[ "deprecation_warning" ] = DeprecationWarning();

  base_data[ "node_id_" ] = zero_index_instance;
  base_data[ "thread_lid_" ] = zero_index_instance;
  base_data[ "model_id_" ] = -1;
  base_data[ "thread_" ] = zero_thread_instance;
  base_data[ "vp_" ] = invalid_thread;
  base_data[ "frozen_" ] = false;
  base_data[ "initialized_" ] = false;
  base_data[ "node_uses_wfr_" ] = false;
  base_data[ "nc_ptr_" ] = nullptr;

  data.push_back( base_data );
}


index
Node::get_node_id() const
{
  index res = invalid_index;
  boost::any node_id_ = data.at( 0 ).at( "node_id_" );
  try
  {
    node_id_ = res = boost::any_cast< index >( node_id_ );
  }
  catch ( const boost::bad_any_cast& e )
  {
    LOG( M_ERROR, "Node::get_node_id", e.what() );
    assert( node_id_.type() == typeid( index ) );
  }
  return res;
}


void
Node::init_state_()
{
}

void
Node::init()
{
  boost::any& initialized_ref = this->data.at( 0 ).at( "initialized_" );
  bool initialized_ = boost::any_cast< bool >( initialized_ref );
  if ( initialized_ )
  {
    return;
  }

  init_state_();
  init_buffers_();

  initialized_ref = true;
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
  int model_id_ = boost::any_cast< int >( this->data.at( 0 ).at( "model_id_" ) );
  if ( model_id_ < 0 )
  {
    return std::string( "UnknownNode" );
  }

  return kernel().model_manager.get_node_model( model_id_ )->get_name();
}

Model&
Node::get_model_() const
{

  int model_id_ = boost::any_cast< int >( this->data.at( 0 ).at( "model_id_" ) );
  assert( model_id_ >= 0 );
  return *kernel().model_manager.get_node_model( model_id_ );
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

  boost::any& frozen = this->data.at( 0 ).at( "frozen_" );
  updateValue< bool >( dict, names::frozen, frozen );
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


void
Node::handle( WeightRecorderEvent& )
{
  throw UnexpectedEvent( "The target node does not handle weight recorder events." );
}


void
Node::handle( RateEvent& )
{
  throw UnexpectedEvent( "The target node does not handle rate input." );
}


void
Node::handle( CurrentEvent& )
{
  throw UnexpectedEvent( "The target node does not handle current input." );
}


void
Node::handle( DataLoggingRequest& )
{
  throw UnexpectedEvent( "The target node does not handle data logging requests." );
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


void
Node::handle( DoubleDataEvent& )
{
  throw UnexpectedEvent();
}


void
Node::handle( GapJunctionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle gap junction input." );
}


void
Node::handle( InstantaneousRateConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle instantaneous rate input." );
}


void
Node::handle( DiffusionConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle diffusion input." );
}


void
Node::handle( DelayedRateConnectionEvent& )
{
  throw UnexpectedEvent( "The target node does not handle delayed rate input." );
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
