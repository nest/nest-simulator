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
 * Connections are unlabeled by default. Unlabeled connections cannot be
 * specified
 * as a search criterion in the `GetConnections` function.
 * @see ConnectionLabel
 */
const static long UNLABELED_CONNECTION = -1;

/**
 * The class ConnectionLabel enables synapse model to be labeled by a positive
 * integer. The label can be set / retrieved with the `names::synapse_label`
 * property in the parameter dictionary of `Set/GetStatus` or `Connect`.
 * Using the `GetConnections` function, synapses with the same label can be
 * specified.
 *
 * The name of synapse models, which can be labeled, end with '_lbl'.
 * @see nest::ConnectionManager::get_connections
 */
template < typename ConnectionT >
class ConnectionLabel : public ConnectionT
{
public:
  ConnectionLabel();

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   *
   * @note Target and Rport cannot be changed after a connection has been
   * created.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  long get_label() const;

private:
  long label_;
};

template < typename ConnectionT >
ConnectionLabel< ConnectionT >::ConnectionLabel()
  : ConnectionT()
  , label_( UNLABELED_CONNECTION )
{
}

template < typename ConnectionT >
void
ConnectionLabel< ConnectionT >::get_status( DictionaryDatum& d ) const
{
  ConnectionT::get_status( d );
  def< long >( d, names::synapse_label, label_ );
  // override names::size_of from ConnectionT,
  // as the size from ConnectionLabel< ConnectionT > is
  // one long larger
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename ConnectionT >
void
ConnectionLabel< ConnectionT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  long lbl;
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
  ConnectionT::set_status( d, cm );
}

template < typename ConnectionT >
inline long
ConnectionLabel< ConnectionT >::get_label() const
{
  return label_;
}


} // namespace nest


#endif /* CONNECTION_LABEL_H */
