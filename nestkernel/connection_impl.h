/*
 *  connection_impl.h
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

#ifndef CONNECTION_IMPL_H
#define CONNECTION_IMPL_H

#include "connection.h"

#include "connection_manager.h"
#include "delay_checker.h"
#include "nest_timeconverter.h"

namespace nest
{

template < typename targetidentifierT >
void
Connection< targetidentifierT >::check_connection_( Node& dummy_target,
  Node& source,
  Node& target,
  const size_t receptor_type )
{
  // 1. does this connection support the event type sent by source
  // try to send event from source to dummy_target
  // this line might throw an exception
  source.send_test_event( dummy_target, receptor_type, get_syn_id(), true );

  // 2. does the target accept the event type sent by source
  // try to send event from source to target
  // this returns the port of the incoming connection
  // p must be stored in the base class connection
  // this line might throw an exception
  target_.set_rport( source.send_test_event( target, receptor_type, get_syn_id(), false ) );

  // 3. do the events sent by source mean the same thing as they are
  // interpreted in target?
  // note that we here use a bitwise and operation (&), because we interpret
  // each bit in the signal type as a collection of individual flags
  if ( not( source.sends_signal() & target.receives_signal() ) )
  {
    throw IllegalConnection( "Source and target neuron are not compatible (e.g., spiking vs binary neuron)." );
  }

  target_.set_target( &target );
}

template < typename targetidentifierT >
void
Connection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  def< double >( d, names::delay, syn_id_delay_.get_delay_ms() );
  target_.get_status( d );
}

template < typename targetidentifierT >
void
Connection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& )
{
  double delay;
  if ( updateValue< double >( d, names::delay, delay ) )
  {
    kernel::manager< ConnectionManager >.get_delay_checker().assert_valid_delay_ms( delay );
    syn_id_delay_.set_delay_ms( delay );
  }
  // no call to target_.set_status() because target and rport cannot be changed
}

template < typename targetidentifierT >
void
Connection< targetidentifierT >::check_synapse_params( const DictionaryDatum& ) const
{
}

template < typename targetidentifierT >
void
Connection< targetidentifierT >::calibrate( const TimeConverter& tc )
{
  Time t = tc.from_old_steps( syn_id_delay_.delay );
  syn_id_delay_.delay = t.get_steps();

  if ( syn_id_delay_.delay == 0 )
  {
    syn_id_delay_.delay = 1;
  }
}

template < typename targetidentifierT >
void
Connection< targetidentifierT >::trigger_update_weight( const size_t,
  const std::vector< spikecounter >&,
  const double,
  const CommonSynapseProperties& )
{
  throw IllegalConnection( "Connection does not support updates that are triggered by a volume transmitter." );
}

template < typename targetidentifierT >
std::unique_ptr< SecondaryEvent >
Connection< targetidentifierT >::get_secondary_event()
{
  assert( false and "Non-primary connections have to provide get_secondary_event()" );
  return nullptr;
}
} // namespace nest

#endif /* CONNECTION_IMPL_H */
