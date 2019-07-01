/*
 *  proxynode.cpp
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

#include "proxynode.h"

// Includes from nestkernel:
#include "connection.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"


namespace nest
{

proxynode::proxynode( index gid, index model_id, index vp )
  : Node()
{
  set_gid_( gid );
  set_model_id( model_id );
  set_vp( vp );
  set_frozen_( true );
}

port
proxynode::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  return kernel()
    .model_manager.get_model( get_model_id() )
    ->send_test_event( target, receptor_type, syn_id, dummy_target );
}

void
proxynode::sends_secondary_event( GapJunctionEvent& ge )
{
  kernel().model_manager.get_model( get_model_id() )->sends_secondary_event( ge );
}

void
proxynode::sends_secondary_event( InstantaneousRateConnectionEvent& re )
{
  kernel().model_manager.get_model( get_model_id() )->sends_secondary_event( re );
}

void
proxynode::sends_secondary_event( DiffusionConnectionEvent& de )
{
  kernel().model_manager.get_model( get_model_id() )->sends_secondary_event( de );
}

void
proxynode::sends_secondary_event( DelayedRateConnectionEvent& re )
{
  kernel().model_manager.get_model( get_model_id() )->sends_secondary_event( re );
}

/**
 * @returns type of signal this node produces
 * used in check_connection to only connect neurons which send / receive
 * compatible information
 * delgates to underlying model
 */
nest::SignalType
proxynode::sends_signal() const
{
  return kernel().model_manager.get_model( get_model_id() )->sends_signal();
}

void
proxynode::get_status( DictionaryDatum& d ) const
{
  const Model* model = kernel().model_manager.get_model( model_id_ );
  const Name element_type = model->get_prototype().get_element_type();
  ( *d )[ names::element_type ] = LiteralDatum( element_type );
}


} // namespace
