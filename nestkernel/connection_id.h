/*
 *  connection_id.h
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

#ifndef CONNECTION_ID_H
#define CONNECTION_ID_H

// Includes from sli:
#include "arraydatum.h"
#include "dictutils.h"

namespace nest
{

class ConnectionID
{
public:
  ConnectionID();
  ConnectionID( long source_node_id, long target_node_id, long target_thread, long synapse_modelid, long port );
  ConnectionID( long source_node_id, long target_thread, long synapse_modelid, long port );
  ConnectionID( const ConnectionID& );

  DictionaryDatum get_dict() const;
  ArrayDatum to_ArrayDatum() const;
  bool operator==( const ConnectionID& c ) const;
  void print_me( std::ostream& out ) const;
  long get_source_node_id() const;
  long get_target_node_id() const;
  long get_target_thread() const;
  long get_synapse_model_id() const;
  long get_port() const;

protected:
  long source_node_id_;
  long target_node_id_;
  long target_thread_;
  long synapse_modelid_;
  long port_;
};

inline ConnectionID::ConnectionID()
  : source_node_id_( -1 )
  , target_node_id_( -1 )
  , target_thread_( -1 )
  , synapse_modelid_( -1 )
  , port_( -1 )
{
}

inline ConnectionID::ConnectionID( const ConnectionID& cid )
  : source_node_id_( cid.source_node_id_ )
  , target_node_id_( cid.target_node_id_ )
  , target_thread_( cid.target_thread_ )
  , synapse_modelid_( cid.synapse_modelid_ )
  , port_( cid.port_ )
{
}

inline long
ConnectionID::get_source_node_id() const
{
  return source_node_id_;
}

inline long
ConnectionID::get_target_node_id() const
{
  return target_node_id_;
}

inline long
ConnectionID::get_target_thread() const
{
  return target_thread_;
}

inline long
ConnectionID::get_synapse_model_id() const
{
  return synapse_modelid_;
}

inline long
ConnectionID::get_port() const
{
  return port_;
}

} // namespace

#endif /* #ifndef CONNECTION_ID_H */
