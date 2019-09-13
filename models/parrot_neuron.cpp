/*
 *  parrot_neuron.cpp
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


#include "parrot_neuron.h"

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

parrot_neuron::parrot_neuron()
  : Archiving_Node()
{
}

void
parrot_neuron::init_buffers_()
{
  B_.n_spikes_.clear(); // includes resize
  Archiving_Node::clear_history();
}

void
parrot_neuron::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    const unsigned long current_spikes_n = static_cast< unsigned long >( B_.n_spikes_.get_value( lag ) );
    if ( current_spikes_n > 0 )
    {
      // create a new SpikeEvent, set its multiplicity and send it
      SpikeEvent se;
      se.set_multiplicity( current_spikes_n );
      kernel().event_delivery_manager.send( *this, se, lag );

      // set the spike times, respecting the multiplicity
      for ( unsigned long i = 0; i < current_spikes_n; i++ )
      {
        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      }
    }
  }
}

void
parrot_neuron::get_status( DictionaryDatum& d ) const
{
  def< double >( d, names::t_spike, get_spiketime_ms() );
  Archiving_Node::get_status( d );
}

void
parrot_neuron::set_status( const DictionaryDatum& d )
{
  Archiving_Node::set_status( d );
}

void
parrot_neuron::handle( SpikeEvent& e )
{
  // Repeat only spikes incoming on port 0, port 1 will be ignored
  if ( 0 == e.get_rport() )
  {
    B_.n_spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      static_cast< double >( e.get_multiplicity() ) );
  }
}

} // namespace
