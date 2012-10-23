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

#include "exceptions.h"
#include "parrot_neuron_ps.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"

#include <limits>

namespace nest
{

parrot_neuron_ps::parrot_neuron_ps()
  : Node()
{}

void parrot_neuron_ps::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
}

void parrot_neuron_ps::update(Time const &origin, 
			      long_t const from, long_t const to)
{
  assert ( to >= 0 );
  assert ( static_cast<delay>(from) < Scheduler::get_min_delay() );
  assert ( from < to );

  // at start of slice, tell input queue to prepare for delivery
  if ( from == 0 )
    B_.events_.prepare_delivery();

  for ( long_t lag = from; lag < to; ++lag )
  {
    // time at start of update step
    long_t const T = origin.get_steps() + lag;

    double_t ev_offset;
    double_t ev_weight;
    bool     end_of_refract;

    while ( B_.events_.get_next_spike(T, ev_offset, ev_weight, end_of_refract) )
    {
      // send spike
      SpikeEvent se;
      se.set_offset(ev_offset);
      network()->send(*this, se, lag);
    }
  }
}                           

// function handles exact spike times
void parrot_neuron_ps::handle(SpikeEvent &e)
{
  assert ( e.get_delay() > 0 );

  // We need to compute the absolute time stamp of the delivery time
  // of the spike, since spikes might spend longer than min_delay_
  // in the queue.  The time is computed according to Time Memo, Rule 3.
  long_t const Tdeliver = e.get_stamp().get_steps() + e.get_delay() - 1;

  B_.events_.add_spike(e.get_rel_delivery_steps(network()->get_slice_origin()),
                       Tdeliver, e.get_offset(), e.get_weight() * e.get_multiplicity());
}

} // namespace
