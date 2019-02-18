/*
 *  parrot_neuron_ps.cpp
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

#include "parrot_neuron_ps.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{

parrot_neuron_ps::parrot_neuron_ps()
  : Archiving_Node()
{
}

void
parrot_neuron_ps::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
  Archiving_Node::clear_history();
}

void
parrot_neuron_ps::update( Time const& origin, long const from, long const to )
{
  assert( to >= 0 );
  assert( static_cast< delay >( from )
    < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // at start of slice, tell input queue to prepare for delivery
  if ( from == 0 )
  {
    B_.events_.prepare_delivery();
  }

  for ( long lag = from; lag < to; ++lag )
  {
    // time at start of update step
    long const T = origin.get_steps() + lag;

    double ev_offset;
    double ev_multiplicity; // parrot stores multiplicity in weight
    bool end_of_refract;

    while ( B_.events_.get_next_spike(
      T, false, ev_offset, ev_multiplicity, end_of_refract ) )
    {
      const unsigned long multiplicity =
        static_cast< unsigned long >( ev_multiplicity );

      // send spike
      SpikeEvent se;
      se.set_multiplicity( multiplicity );
      se.set_offset( ev_offset );
      kernel().event_delivery_manager.send( *this, se, lag );

      for ( unsigned long i = 0; i < multiplicity; ++i )
      {
        set_spiketime( Time::step( T + 1 ), ev_offset );
      }
    }
  }
}

void
parrot_neuron_ps::get_status( DictionaryDatum& d ) const
{
  Archiving_Node::get_status( d );
}

void
parrot_neuron_ps::set_status( const DictionaryDatum& d )
{
  Archiving_Node::set_status( d );
}

// function handles exact spike times
void
parrot_neuron_ps::handle( SpikeEvent& e )
{
  // Repeat only spikes incoming on port 0, port 1 will be ignored
  if ( 0 == e.get_rport() )
  {
    assert( e.get_delay_steps() > 0 );

    // We need to compute the absolute time stamp of the delivery time
    // of the spike, since spikes might spend longer than min_delay_
    // in the queue.  The time is computed according to Time Memo, Rule 3.
    const long Tdeliver = e.get_stamp().get_steps() + e.get_delay_steps() - 1;

    // parrot ignores weight of incoming connection, store multiplicity
    B_.events_.add_spike(
      e.get_rel_delivery_steps(
        nest::kernel().simulation_manager.get_slice_origin() ),
      Tdeliver,
      e.get_offset(),
      static_cast< double >( e.get_multiplicity() ) );
  }
}

} // namespace
