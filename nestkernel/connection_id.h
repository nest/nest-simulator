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

// Includes from libnestutil:
#include "config.h"

#include "dictionary.h"


namespace nest
{

class ConnectionID
{
public:
  ConnectionID();
  ConnectionID( long source_node_id, long target_node_id, long target_thread, long synapse_modelid, long port );
  ConnectionID( long source_node_id, long target_thread, long synapse_modelid, long port );
  ConnectionID( const ConnectionID& ) = default;

  Dictionary get_dict() const;
  bool operator==( const ConnectionID& c ) const;
  void print_me( std::ostream& out ) const;
  long get_source_node_id() const;
  long get_target_node_id() const;
  long get_target_thread() const;
  long get_synapse_model_id() const;
  long get_port() const;

  /**
   * Return true if the connection is on the long path of morph.
   *
   * Without morph, all connections are on the regular path and the function returns false.
   */
  bool is_long_delay() const;

#ifdef HAVE_MORPH
  //! Mark the connection as being on the long path of morph.
  void set_long_delay();
#endif

protected:
  long source_node_id_;
  long target_node_id_;
  long target_thread_;
  long synapse_modelid_;
  long port_;
#ifdef HAVE_MORPH
  bool is_long_delay_;  //!< the connection is on the long path of morph; port_ is an index into its tables
#endif
};

inline ConnectionID::ConnectionID()
  : source_node_id_( -1 )
  , target_node_id_( -1 )
  , target_thread_( -1 )
  , synapse_modelid_( -1 )
  , port_( -1 )
#ifdef HAVE_MORPH
  , is_long_delay_( false )
#endif
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

inline bool
ConnectionID::is_long_delay() const
{
#ifdef HAVE_MORPH
  return is_long_delay_;
#else
  return false;
#endif
}

#ifdef HAVE_MORPH
inline void
ConnectionID::set_long_delay()
{
  is_long_delay_ = true;
}
#endif

}  // namespace

#endif /* #ifndef CONNECTION_ID_H */
