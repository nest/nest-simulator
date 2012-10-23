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
#include "connector.h"
#include "spikecounter.h"

#include <numeric>

/* ---------------------------------------------------------------- 
 * Default constructor defining default parameters 
 * ---------------------------------------------------------------- */


nest::volume_transmitter::Parameters_::Parameters_()
  : deliver_interval_ (1.0) //in d_min time steps
{}

/* ---------------------------------------------------------------- 
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void nest::volume_transmitter::Parameters_::get(DictionaryDatum &d) const
{
  def<long_t>(d, "deliver_interval", deliver_interval_);
}

void::nest::volume_transmitter::Parameters_::set(const DictionaryDatum& d)
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

nest::volume_transmitter::volume_transmitter(const volume_transmitter &n)
  : Archiving_Node(n),
    P_(n.P_)
{
}

void nest::volume_transmitter::init_state_(const Node&)
{
}

void nest::volume_transmitter::init_buffers_()
{ 
  B_.neuromodulatory_spikes_.clear();
  B_.spikecounter_.clear();
  Archiving_Node::clear_history();
}




void nest::volume_transmitter::register_connector(Connector& c)
{
  B_.targets_.push_back(&c);
}

void nest::volume_transmitter::calibrate()
{
  V_.counter_ = 0;
  B_.spikecounter_.reserve(Scheduler::get_min_delay()*P_.deliver_interval_);
}

void nest::volume_transmitter::update(Time const&, const long_t from, const long_t to)
{
   V_.counter_ = V_.counter_ + 1;
   double_t h = Time::get_resolution().get_ms();
   double_t neuromodulatory_spike;
   double_t multiplicity;

  //all spikes stored in spikecounter_ are delivered to the target synapses
  if(V_.counter_ == P_.deliver_interval_)
    {
      if (B_.spikecounter_.size() > 0)
	{
	  for (nest::index t = 0; t < B_.targets_.size(); ++t)
	    {
	      B_.targets_[t]->trigger_update_weight(B_.spikecounter_); 
	    }
	}
  
      B_.spikecounter_.clear();
      V_.counter_=0;
    }

  //spikes arriving in future time slices are stored in spikecounter_
  for (long_t lag = from; lag < to; ++lag)
    {
      multiplicity = B_.neuromodulatory_spikes_.get_value(lag);
      if(multiplicity !=0)
	{
	  neuromodulatory_spike = (network()->get_slice_origin().get_steps() + lag + 1)*h; //neuromodulatory spikes are delivered with delay time
	  B_.spikecounter_.push_back(spikecounter(neuromodulatory_spike, multiplicity));
	  	}
    }
} 



void nest::volume_transmitter::handle(SpikeEvent & e)
{
  B_.neuromodulatory_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), (double_t) e.get_multiplicity());
}
