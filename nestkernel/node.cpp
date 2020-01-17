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
  , vp_( invalid_thread_ )
  , frozen_( false )
  , buffers_initialized_( false )
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
  , buffers_initialized_( false )
  , node_uses_wfr_( n.node_uses_wfr_ )
{
}

Node::~Node()
{
}

void
Node::init_state()
{
  Model const* const model = kernel().model_manager.get_model( model_id_ );
  assert( model );
  init_state_( model->get_prototype() );
}

void
Node::init_state_( Node const& )
{
}

void
Node::init_buffers()
{
  if ( buffers_initialized_ )
  {
    return;
  }

  init_buffers_();

  buffers_initialized_ = true;
}

void
Node::init_buffers_()
{
}

void
Node::set_initialized()
{
  set_initialized_();
  initialized_ = true;
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

  return kernel().model_manager.get_model( model_id_ )->get_name();
}

Model&
Node::get_model_() const
{
  if ( model_id_ < 0 )
  {
    throw UnknownModelID( model_id_ );
  }

  return *kernel().model_manager.get_model( model_id_ );
}

DictionaryDatum
Node::get_status_dict_()
{
  return DictionaryDatum( new Dictionary );
}

void
Node::set_local_device_id( const index lsdid )
{
  assert( false && "set_local_device_id() called on a non-device node of type" );
}

index
Node::get_local_device_id() const
{
  assert( false && "set_local_device_id() called on a non-device node." );
}

DictionaryDatum
Node::get_status_base()
{
  DictionaryDatum dict = get_status_dict_();

  // add information available for all nodes
  ( *dict )[ names::local ] = kernel().node_manager.is_local_node( this );
  ( *dict )[ names::model ] = LiteralDatum( get_name() );
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
  throw UnexpectedEvent();
}

/**
 * Default implementation of check_connection just throws UnexpectedEvent
 */
port
Node::send_test_event( Node&, rport, synindex, bool )
{
  throw UnexpectedEvent();
}

/**
 * Default implementation of register_stdp_connection() just
 * throws IllegalConnection
 */
void
Node::register_stdp_connection( double, double )
{
  throw IllegalConnection();
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
  throw UnexpectedEvent();
}

port
Node::handles_test_event( SpikeEvent&, rport )
{
  throw IllegalConnection();
}

void
Node::handle( WeightRecorderEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( WeightRecorderEvent&, rport )
{
  throw IllegalConnection();
}

void
Node::handle( RateEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( RateEvent&, rport )
{
  throw IllegalConnection();
}

void
Node::handle( CurrentEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( CurrentEvent&, rport )
{
  throw IllegalConnection();
}

void
Node::handle( DataLoggingRequest& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( DataLoggingRequest&, rport )
{
  throw IllegalConnection(
    "Possible cause: only static synapse types may be used to connect "
    "devices." );
}

void
Node::handle( DataLoggingReply& )
{
  throw UnexpectedEvent();
}

void
Node::handle( ConductanceEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( ConductanceEvent&, rport )
{
  throw IllegalConnection();
}

void
Node::handle( DoubleDataEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( DoubleDataEvent&, rport )
{
  throw IllegalConnection();
}

port
Node::handles_test_event( DSSpikeEvent&, rport )
{
  throw IllegalConnection(
    "Possible cause: only static synapse types may be used to connect "
    "devices." );
}

port
Node::handles_test_event( DSCurrentEvent&, rport )
{
  throw IllegalConnection(
    "Possible cause: only static synapse types may be used to connect "
    "devices." );
}

void
Node::handle( GapJunctionEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( GapJunctionEvent&, rport )
{
  throw IllegalConnection();
  return invalid_port_;
}

void
Node::sends_secondary_event( GapJunctionEvent& )
{
  throw IllegalConnection();
}

void
Node::handle( InstantaneousRateConnectionEvent& )
{
  throw UnexpectedEvent();
}

void
Node::handle( DiffusionConnectionEvent& )
{
  throw UnexpectedEvent();
}

void
Node::handle( DelayedRateConnectionEvent& )
{
  throw UnexpectedEvent();
}

port
Node::handles_test_event( InstantaneousRateConnectionEvent&, rport )
{
  throw IllegalConnection();
  return invalid_port_;
}

port
Node::handles_test_event( DiffusionConnectionEvent&, rport )
{
  throw IllegalConnection();
  return invalid_port_;
}

port
Node::handles_test_event( DelayedRateConnectionEvent&, rport )
{
  throw IllegalConnection();
  return invalid_port_;
}

void
Node::sends_secondary_event( InstantaneousRateConnectionEvent& )
{
  throw IllegalConnection();
}

void
Node::sends_secondary_event( DiffusionConnectionEvent& )
{
  throw IllegalConnection();
}

void
Node::sends_secondary_event( DelayedRateConnectionEvent& )
{
  throw IllegalConnection();
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
  std::deque< histentry_cl >::iterator*,
  std::deque< histentry_cl >::iterator* )
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
