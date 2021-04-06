/*
 *  connection_id.cpp
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

#include "connection_id.h"

// Includes from nestkernel:
#include "nest_names.h"

// Includes from sli:
#include "integerdatum.h"

namespace nest
{

ConnectionID::ConnectionID( long source_node_id,
  long target_node_id,
  long target_thread,
  long synapse_modelid,
  long port )
  : source_node_id_( source_node_id )
  , target_node_id_( target_node_id )
  , target_thread_( target_thread )
  , synapse_modelid_( synapse_modelid )
  , port_( port )
{
}

ConnectionID::ConnectionID( long source_node_id, long target_thread, long synapse_modelid, long port )
  : source_node_id_( source_node_id )
  , target_node_id_( -1 )
  , target_thread_( target_thread )
  , synapse_modelid_( synapse_modelid )
  , port_( port )
{
}

DictionaryDatum
ConnectionID::get_dict() const
{
  DictionaryDatum dict( new Dictionary );

  // The node ID of the presynaptic node
  def< long >( dict, nest::names::source, source_node_id_ );
  // The node ID of the postsynaptic node
  def< long >( dict, nest::names::target, target_node_id_ );
  // The id of the synapse model
  def< long >( dict, nest::names::synapse_modelid, synapse_modelid_ );
  // The thread of the postsynaptic node
  def< long >( dict, nest::names::target_thread, target_thread_ );
  // The index in the list
  def< long >( dict, nest::names::port, port_ );

  return dict;
}

ArrayDatum
ConnectionID::to_ArrayDatum() const
{
  ArrayDatum ad;
  ad.push_back( new IntegerDatum( source_node_id_ ) );
  ad.push_back( new IntegerDatum( target_node_id_ ) );
  ad.push_back( new IntegerDatum( target_thread_ ) );
  ad.push_back( new IntegerDatum( synapse_modelid_ ) );
  ad.push_back( new IntegerDatum( port_ ) );
  return ad;
}

bool ConnectionID::operator==( const ConnectionID& c ) const
{
  return ( source_node_id_ == c.source_node_id_ ) and ( target_node_id_ == c.target_node_id_ )
    and ( target_thread_ == c.target_thread_ ) and ( port_ == c.port_ ) and ( synapse_modelid_ == c.synapse_modelid_ );
}

void
ConnectionID::print_me( std::ostream& out ) const
{
  out << "<" << source_node_id_ << "," << target_node_id_ << "," << target_thread_ << "," << synapse_modelid_ << ","
      << port_ << ">";
}

} // namespace
