/*
 *  volume_transmitter.cpp
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
#include "volume_transmitter.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "arraydatum.h"
#include "connector_base.h"
#include "spikecounter.h"

#include <numeric>

/* ----------------------------------------------------------------
 * Default constructor defining default parameters
 * ---------------------------------------------------------------- */

nest::volume_transmitter::Parameters_::Parameters_()
  : deliver_interval_(1) // in steps of mindelay
{}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void nest::volume_transmitter::Parameters_::get(DictionaryDatum & d) const
{
  def<long_t>(d, "deliver_interval", deliver_interval_);
}

void::nest::volume_transmitter::Parameters_::set(const DictionaryDatum & d)
{
  updateValue<long_t>(d, "deliver_interval", deliver_interval_);
}

/* ----------------------------------------------------------------
 * Default and copy constructor for volume transmitter
 * ---------------------------------------------------------------- */

nest::volume_transmitter::volume_transmitter()
  : Archiving_Node(),
    P_()
{}

nest::volume_transmitter::volume_transmitter(const volume_transmitter & n)
  : Archiving_Node(n),
    P_(n.P_)
{}

void nest::volume_transmitter::init_state_(const Node &)
{}

void nest::volume_transmitter::init_buffers_()
{
  B_.neuromodulatory_spikes_.clear();
  B_.spikecounter_.clear();
  B_.spikecounter_.push_back(spikecounter(0.0, 0.0));  // insert pseudo last dopa spike at t = 0.0
  Archiving_Node::clear_history();
}

void nest::volume_transmitter::calibrate()
{
  // +1 as pseudo dopa spike at t_trig is inserted after trigger_update_weight
  B_.spikecounter_.reserve(Scheduler::get_min_delay()*P_.deliver_interval_+1);
}

void nest::volume_transmitter::update(const Time&, const long_t from, const long_t to)
{
  // spikes that arrive in this time slice are stored in spikecounter_
  double_t t_spike;
  double_t multiplicity;
  for ( long_t lag = from; lag < to; ++lag )
  {
    multiplicity = B_.neuromodulatory_spikes_.get_value(lag);
    if ( multiplicity > 0 )
    {
      t_spike = Time(Time::step(network()->get_slice_origin().get_steps() + lag + 1)).get_ms();
      B_.spikecounter_.push_back(spikecounter(t_spike, multiplicity));
    }
  }

  // all spikes stored in spikecounter_ are delivered to the target synapses
  if ( ( network()->get_slice_origin().get_steps() + to ) % ( P_.deliver_interval_ * Scheduler::get_min_delay() ) == 0 )
  {
    double_t t_trig = Time(Time::step(network()->get_slice_origin().get_steps() + to)).get_ms();

    if ( !B_.spikecounter_.empty() )
      network()->trigger_update_weight(get_gid(), B_.spikecounter_, t_trig);

    // clear spikecounter
    B_.spikecounter_.clear();

    // as with trigger_update_weight dopamine trace has been updated to t_trig, insert pseudo last dopa spike at t_trig
    B_.spikecounter_.push_back(spikecounter(t_trig, 0.0));
  }
}

void nest::volume_transmitter::handle(SpikeEvent& e)
{
  B_.neuromodulatory_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
				       static_cast<double_t>(e.get_multiplicity()));
}
