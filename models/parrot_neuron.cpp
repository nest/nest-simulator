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

#include "exceptions.h"
#include "parrot_neuron.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"

#include <limits>
namespace nest
{

parrot_neuron::parrot_neuron()
  : Archiving_Node()
{}

void parrot_neuron::init_buffers_()
{
  B_.n_spikes_.clear();  // includes resize
  Archiving_Node::clear_history();
}

void parrot_neuron::update(Time const & origin, 
			   const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);

  SpikeEvent se;

  for ( long_t lag = from ; lag < to ; ++lag )
  {
    const ulong_t current_spikes_n
      = static_cast<ulong_t>(B_.n_spikes_.get_value(lag));      

    if (current_spikes_n > 0)
    {
      for (ulong_t i_spike=0; i_spike < current_spikes_n; i_spike++)
        network()->send(*this, se, lag);
      set_spiketime(Time::step(origin.get_steps()+lag+1));
    }
  }
}                           

void parrot_neuron::get_status(DictionaryDatum &d) const
{
  def<double>(d, names::t_spike, get_spiketime_ms());
}

void parrot_neuron::set_status(const DictionaryDatum& d)
{
  Archiving_Node::set_status(d);
}

void parrot_neuron::handle(SpikeEvent & e)
{
  B_.n_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                         static_cast<double_t>(e.get_multiplicity()));
}
 
} // namespace
