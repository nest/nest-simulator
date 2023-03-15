/*
 *  nest_object_interface.cpp
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


#include "nest_object_interface.h"
#include "kernel_manager.h"

namespace nest
{


NESTObjectInterface::NESTObjectInterface()
  : node_id_( 0 )
  , thread_lid_( invalid_index )
  , model_id_( -1 )
  , thread_( invalid_thread )
  , vp_( invalid_thread )
{
}


NESTObjectInterface::NESTObjectInterface( const NESTObjectInterface& n )
  : node_id_( 0 )
  , thread_lid_( n.thread_lid_ )
  , model_id_( n.model_id_ )
  , thread_( n.thread_ )
  , vp_( n.vp_ )
{
}

NESTObjectInterface::~NESTObjectInterface()
{
}

std::string
NESTObjectInterface::get_name() const
{
  if ( model_id_ < 0 )
  {
    return std::string( "UnknownNode" );
  }

  return kernel().model_manager.get_node_model( model_id_ )->get_name();
}


DictionaryDatum
NESTObjectInterface::get_status_dict_()
{
  return DictionaryDatum( new Dictionary );
}


void
NESTObjectInterface::set_local_device_id( const index )
{
  assert( false and "set_local_device_id() called on a non-device node of type" );
}

index
NESTObjectInterface::get_local_device_id() const
{
  assert( false and "get_local_device_id() called on a non-device node." );
  return invalid_index;
}

DictionaryDatum
NESTObjectInterface::get_status_base()
{
  DictionaryDatum dict = get_status_dict_();

  // add information available for all nodes
  ( *dict )[ names::model ] = LiteralDatum( get_name() );
  ( *dict )[ names::model_id ] = get_model_id();
  ( *dict )[ names::global_id ] = get_node_id();
  ( *dict )[ names::vp ] = get_vp();
  ( *dict )[ names::element_type ] = LiteralDatum( get_element_type() );

  return dict;
}


void
NESTObjectInterface::set_status_base( const DictionaryDatum& dict )
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
}

}