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
