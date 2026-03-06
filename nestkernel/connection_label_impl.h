/*
 *  connection_label_impl.h
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

#ifndef CONNECTION_LABEL_IMPL_H
#define CONNECTION_LABEL_IMPL_H

#include "exceptions.h"
#include "nest_names.h"

#include <connection_label.h>

namespace nest
{

template < typename ConnectionT >
ConnectionLabel< ConnectionT >::ConnectionLabel()
  : ConnectionT()
  , label_( UNLABELED_CONNECTION )
{
}

template < typename ConnectionT >
void
ConnectionLabel< ConnectionT >::get_status( Dictionary& d ) const
{
  ConnectionT::get_status( d );
  d[ names::synapse_label ] = label_;
  // override names::size_of from ConnectionT,
  // as the size from ConnectionLabel< ConnectionT > is
  // one long larger
  d[ names::size_of ] = sizeof( *this );
}

template < typename ConnectionT >
void
ConnectionLabel< ConnectionT >::set_status( const Dictionary& d, ConnectorModel& cm )
{
  long lbl;
  if ( d.update_integer_value( names::synapse_label, lbl ) )
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
}  // namespace nest


#endif /* CONNECTION_LABEL_IMPL_H */
