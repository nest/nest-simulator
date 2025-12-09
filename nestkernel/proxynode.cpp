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
#include "model_manager.h"

// Includes from sli:
#include "dictutils.h"


namespace nest
{

proxynode::proxynode( size_t node_id, size_t model_id, size_t vp )
  : Node()
{
  set_node_id_( node_id );
  set_model_id( model_id );
  set_vp( vp );
  set_frozen_( true );
}

size_t
proxynode::send_test_event( Node& target, size_t receptor_type, synindex syn_id, bool dummy_target )
{
  Model* model = kernel::manager< ModelManager >.get_node_model( get_model_id() );
  return model->send_test_event( target, receptor_type, syn_id, dummy_target );
}

void
proxynode::sends_secondary_event( GapJunctionEvent& ge )
{
  kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_secondary_event( ge );
}

void
proxynode::sends_secondary_event( InstantaneousRateConnectionEvent& re )
{
  kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_secondary_event( re );
}

void
proxynode::sends_secondary_event( DiffusionConnectionEvent& de )
{
  kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_secondary_event( de );
}

void
proxynode::sends_secondary_event( DelayedRateConnectionEvent& re )
{
  kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_secondary_event( re );
}

void
proxynode::sends_secondary_event( LearningSignalConnectionEvent& re )
{
  kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_secondary_event( re );
}

void
proxynode::sends_secondary_event( SICEvent& sic )
{
  kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_secondary_event( sic );
}

nest::SignalType
proxynode::sends_signal() const
{
  return kernel::manager< ModelManager >.get_node_model( get_model_id() )->sends_signal();
}

void
proxynode::get_status( DictionaryDatum& d ) const
{
  const Model* model = kernel::manager< ModelManager >.get_node_model( model_id_ );
  const Name element_type = model->get_prototype().get_element_type();
  ( *d )[ names::element_type ] = LiteralDatum( element_type );
}

bool
proxynode::is_proxy() const
{
  return true;
}
} // namespace
