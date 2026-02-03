/*
 *  activation_event_node.cpp
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

#include "activation_event_node.h"

// nestkernel
#include "exceptions.h"
#include "nest_names.h"

// libnestutil
#include "dict_util.h"

namespace nest
{

ActivationEventNode::ActivationEventNode()
  : activation_interval_( 3000.0 )
  , last_event_time_( 0 )
{
}

ActivationEventNode::ActivationEventNode( const ActivationEventNode& n )
  : activation_interval_( n.activation_interval_ )
  , last_event_time_( n.last_event_time_ )
{
}

void
ActivationEventNode::get_status( Dictionary& d ) const
{
  d[ names::activation_interval ] = activation_interval_;
}

void
ActivationEventNode::set_status( const Dictionary& d )
{
  d.update_value( names::activation_interval, activation_interval_ );

  if ( activation_interval_ <= 0.0 )
  {
    throw BadProperty( "Interval between activation events activation_interval > 0 required." );
  }
}

} // namespace nest
