/*
 *  connection_label.h
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

#ifndef CONNECTION_LABEL_H
#define CONNECTION_LABEL_H

#include "nest.h"
#include "nest_names.h"
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{
class ConnectorModel;

/**
 * Connections are unlabeled by default. Unlabeled connections cannot be specified
 * as a search criterion in the `GetConnections` function.
 * @see ConnectionLabel
 */
const long_t UNLABELED_CONNECTION = -1;

/**
 * The class ConnectionLabel enables synapse model to be labeled by a positive integer. The
 * label can be set / retrieved with the `names::synapse_label` property in the parameter
 * dictionary of `Set/GetStatus` or `Connect`.
 * Using the `GetConnections` function, synapses with the same label can be specified.
 *
 * The name of synapse models, which can be labels, end with '_lbl'.
 * @see nest::ConnectionManager::get_connections
 */
template < class Connection_t >
class ConnectionLabel : public Connection_t
{
public:
  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   *
   * @note Target and Rport cannot be changed after a connection has been created.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  long_t get_label() const;

private:
  long_t label_;
};

template < typename Connection_t >
void
ConnectionLabel< Connection_t >::get_status( DictionaryDatum& d ) const
{
  Connection_t::get_status( d );
  def< long_t >( d, names::synapse_label, label_ );
  // override, as the size changes here
  def< long_t >( d, names::size_of, sizeof( *this ) );
}

template < typename Connection_t >
void
ConnectionLabel< Connection_t >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  long_t lbl;
  if ( updateValue< long >( d, names::synapse_label, lbl ) )
  {
    if ( lbl >= 0 )
    {
      label_ = lbl;
    }
    else
    {
      throw BadProperty( "Connection label must not be negative." );
    }
  }
  Connection_t::set_status( d, cm );
}

template < typename Connection_t >
inline long_t
ConnectionLabel< Connection_t >::get_label() const
{
  return label_;
}


} // namespace nest


#endif /* CONNECTION_LABEL_H */
