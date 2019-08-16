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

ConnectionID::ConnectionID( long source_gid, long target_gid, long target_thread, long synapse_modelid, long port )
  : source_gid_( source_gid )
  , target_gid_( target_gid )
  , target_thread_( target_thread )
  , synapse_modelid_( synapse_modelid )
  , port_( port )
{
}

ConnectionID::ConnectionID( long source_gid, long target_thread, long synapse_modelid, long port )
  : source_gid_( source_gid )
  , target_thread_( target_thread )
  , synapse_modelid_( synapse_modelid )
  , port_( port )
{
}

DictionaryDatum
ConnectionID::get_dict() const
{
  DictionaryDatum dict( new Dictionary );

  // The gid of the presynaptic node
  def< long >( dict, nest::names::source, source_gid_ );
  // The gid of the postsynaptic node
  def< long >( dict, nest::names::target, target_gid_ );
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
  ad.push_back( new IntegerDatum( source_gid_ ) );
  ad.push_back( new IntegerDatum( target_gid_ ) );
  ad.push_back( new IntegerDatum( target_thread_ ) );
  ad.push_back( new IntegerDatum( synapse_modelid_ ) );
  ad.push_back( new IntegerDatum( port_ ) );
  return ad;
}

bool ConnectionID::operator==( const ConnectionID& c ) const
{
  return ( source_gid_ == c.source_gid_ ) and ( target_gid_ == c.target_gid_ )
    and ( target_thread_ == c.target_thread_ ) and ( port_ == c.port_ ) and ( synapse_modelid_ == c.synapse_modelid_ );
}

void
ConnectionID::print_me( std::ostream& out ) const
{
  out << "<" << source_gid_ << "," << target_gid_ << "," << target_thread_ << "," << synapse_modelid_ << "," << port_
      << ">";
}

} // namespace
