/*
 *  sli_neuron.cpp
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
#include "sli_neuron.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"
#include "dictstack.h"

#include <limits>

/* ---------------------------------------------------------------- 
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap<nest::sli_neuron> nest::sli_neuron::recordablesMap_;

namespace nest
{
  // Override the create() method with one call to RecordablesMap::insert_() 
  // for each quantity to be recorded.
  template <>
  void RecordablesMap<sli_neuron>::create()
  {
    // use standard names whereever you can for consistency!
    insert_(names::V_m, &sli_neuron::get_V_m_);
  }
}

nest::sli_neuron::Buffers_::Buffers_(sli_neuron &n)
  : logger_(n)
{}

nest::sli_neuron::Buffers_::Buffers_(const Buffers_ &, sli_neuron &n)
  : logger_(n)
{}

/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::sli_neuron::sli_neuron()
  : Archiving_Node(),
    state_(new Dictionary()),
    B_(*this)
{
  // We add empty defaults for /calibrate and /update, so that the uninitialised node runs without errors.
  state_->insert(names::calibrate,new ProcedureDatum());
  state_->insert(names::update,new ProcedureDatum());
  recordablesMap_.create();
}

nest::sli_neuron::sli_neuron(const sli_neuron& n)
  : Archiving_Node(n),
    state_(new Dictionary(*n.state_)),
    B_(n.B_, *this)
{
  init_state_(n);
}

/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::sli_neuron::init_state_(const Node& proto)
{
  const sli_neuron& pr = downcast<sli_neuron>(proto);
  state_= DictionaryDatum(new Dictionary(*pr.state_));
}

void nest::sli_neuron::init_buffers_()
{
  B_.ex_spikes_.clear();       // includes resize
  B_.in_spikes_.clear();       // includes resize
  B_.currents_.clear();        // includes resize
  B_.logger_.reset(); // includes resize
  Archiving_Node::clear_history();
}



void nest::sli_neuron::calibrate()
{
  B_.logger_.init();

  bool terminate=false;

  if(!state_->known(names::calibrate))
    {
      std::string msg=String::compose("Node %1 has no /calibrate function in its status dictionary.",get_gid());
      net_->message(SLIInterpreter::M_ERROR,"sli_neuron::calibrate",msg.c_str());
      terminate=true;
    }

  if(! state_->known(names::update))
    {
      std::string msg=String::compose("Node %1 has no /update function in its status dictionary. Terminating.",get_gid());
      net_->message(SLIInterpreter::M_ERROR,"sli_neuron::calibrate",msg.c_str());
      terminate=true;
    }

  if(terminate)
    {
      net_->terminate();
      net_->message(SLIInterpreter::M_ERROR,"sli_neuron::calibrate","Terminating.");
      return;
    }

  network()->execute_sli_protected(state_, names::calibrate_node);   // call interpreter
}

/* ---------------------------------------------------------------- 
 * Update and spike handling functions
 */
 
void nest::sli_neuron::update(Time const & origin, const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);
  (*state_)[names::t_origin]=origin.get_steps();

  if (state_->known(names::error))
    {
      std::string msg=String::compose("Node %1 still has its error state set.",get_gid());
      net_->message(SLIInterpreter::M_ERROR,"sli_neuron::update",msg.c_str());
      net_->message(SLIInterpreter::M_ERROR,"sli_neuron::update","Please check /calibrate and /update for errors");
      net_->terminate();
      return;
    }

  for ( long_t lag = from ; lag < to ; ++lag )
  {
    (*state_)[names::in_spikes]=B_.in_spikes_.get_value(lag); // in spikes arriving at right border
    (*state_)[names::ex_spikes]=B_.ex_spikes_.get_value(lag); // ex spikes arriving at right border
    (*state_)[names::currents]=B_.currents_.get_value(lag);   
    (*state_)[names::t_lag]=lag;

    network()->execute_sli_protected(state_, names::update_node);   // call interpreter

    bool spike_emission= false;
    if (state_->known(names::spike))
      spike_emission=(*state_)[names::spike];

    // threshold crossing
    if (spike_emission)
    {
      set_spiketime(Time::step(origin.get_steps()+lag+1));
      SpikeEvent se;
      network()->send(*this, se, lag);
    }

    B_.logger_.record_data(origin.get_steps()+lag);
  }  
}                           
                     

void nest::sli_neuron::handle(SpikeEvent & e)
{
  assert(e.get_delay() > 0);

  if(e.get_weight() > 0.0)
    B_.ex_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                            e.get_weight() * e.get_multiplicity() );
  else    
    B_.in_spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                            e.get_weight() * e.get_multiplicity() );
}

void nest::sli_neuron::handle(CurrentEvent& e)
{
  assert(e.get_delay() > 0);

  const double_t I = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), 
		                     w * I);
}

void nest::sli_neuron::handle(DataLoggingRequest& e)
{
  B_.logger_.handle(e);
}
