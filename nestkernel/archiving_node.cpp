/*
 *  archiving_node.cpp
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

/**
 * \file archiving_node.cpp
 * Implementation of archiving_node to record and manage spike history
 * \author Moritz Helias, Abigail Morrison
 * \date april 2006
 */

#include "archiving_node.h"
#include "dictutils.h"

namespace nest {

  //member functions for Archiving_Node   

  nest::Archiving_Node::Archiving_Node() :
    n_incoming_(0),
    Kminus_(0.0),
    triplet_Kminus_(0.0),
    tau_minus_(20.0),
    tau_minus_triplet_(110.0),
    last_spike_(-1.0)
  {}

  nest::Archiving_Node::Archiving_Node(const Archiving_Node& n)
    :Node(n),
     n_incoming_(n.n_incoming_),
     Kminus_(n.Kminus_),
     triplet_Kminus_(n.triplet_Kminus_),
     tau_minus_(n.tau_minus_),
     tau_minus_triplet_(n.tau_minus_triplet_),
     last_spike_(n.last_spike_)
  {}

  void Archiving_Node::register_stdp_connection(double_t t_first_read)
  {
    // Mark all entries in the deque, which we will not read in future as read by this input
    // input, so that we savely increment the incoming number of
    // connections afterwards without leaving spikes in the history.
    // For details see bug #218. MH 08-04-22

    for ( std::deque<histentry>::iterator runner = history_.begin();
	  runner != history_.end() && runner->t_ <= t_first_read;
	  ++runner)
      (runner->access_counter_)++;

    n_incoming_++;    
  }
 
  void Archiving_Node::unregister_stdp_connection(double_t t_last_read)
  {
    // Mark all entries in the deque we have read as unread 
    // so that we can savely decrement the incoming number of
    // connections afterwards without loosing entries, which
    // are still needed. For details see bug #218. MH 08-04-22

    for ( std::deque<histentry>::iterator runner = history_.begin();
	  runner != history_.end() && runner->t_ <= t_last_read;
	  ++runner)
      (runner->access_counter_)--;

    n_incoming_--;
  }

  double_t nest::Archiving_Node::get_K_value(double_t t)
  {
    if (history_.empty()) return Kminus_;
    int i = history_.size() - 1;
    while (i >= 0)
      {
	if (t > history_[i].t_)
	  return (history_[i].Kminus_*std::exp((history_[i].t_ - t)/tau_minus_));
	i--;
      }
    return 0;
  }

  void nest::Archiving_Node::get_K_values(double_t t, double_t& K_value, double_t& triplet_K_value)
  {
    // case when the neuron has not yet spiked
    if (history_.empty()) {
      triplet_K_value = triplet_Kminus_;
      K_value = Kminus_; 
      return;
    }
    // case 
    int i = history_.size() - 1;
    while (i >= 0)
      {
	if (t > history_[i].t_) {
	  triplet_K_value = (history_[i].triplet_Kminus_*std::exp((history_[i].t_ - t)/tau_minus_triplet_));
	  K_value = (history_[i].Kminus_*std::exp((history_[i].t_ - t)/tau_minus_));
	  return;
	}
	i--;
      }

    // we only get here if t< time of all spikes in history)

    // return 0.0 for both K values
    triplet_K_value = 0.0;
    K_value = 0.0;
  }

  void nest::Archiving_Node::get_history(double_t t1, double_t t2,
				   std::deque<histentry>::iterator* start,
				   std::deque<histentry>::iterator* finish)
  {
    *finish = history_.end();
    if (history_.empty())
      {
	*start = *finish;
	return;
      }
    else
      {
	std::deque<histentry>::iterator runner = history_.begin();
	while ((runner != history_.end()) && (runner->t_ <= t1)) ++runner;
	*start = runner;
	while ((runner != history_.end()) && (runner->t_ <= t2))
	  {
	    (runner->access_counter_)++;
	    ++runner;
	  }
	*finish = runner;
      }
  }

  void nest::Archiving_Node::set_spiketime(Time const & t_sp)
  {    
      if (n_incoming_)
      {
	  // prune all spikes from history which are no longer needed
          // except the penultimate one. we might still need it.
	  while (history_.size() > 1)
	  {
	      if (history_.front().access_counter_ >= n_incoming_)
		  history_.pop_front();
	      else
		break;		
	  }
	  // update spiking history
	  Kminus_ = Kminus_ * std::exp((last_spike_ - t_sp.get_ms())/tau_minus_) + 1.0;
	  triplet_Kminus_ = triplet_Kminus_ * std::exp((last_spike_ - t_sp.get_ms())/tau_minus_triplet_) + 1.0;
	  last_spike_ = t_sp.get_ms();
	  history_.push_back( histentry( last_spike_, Kminus_, triplet_Kminus_,0) );
      }
      else
      {
	  last_spike_ = t_sp.get_ms();
      }
  }

  void nest::Archiving_Node::get_status(DictionaryDatum & d) const
  {
    def<double>(d, names::t_spike, get_spiketime_ms());
    def<double>(d, names::tau_minus, tau_minus_);
    def<double>(d, names::tau_minus_triplet, tau_minus_triplet_);
#ifdef DEBUG_ARCHIVER
    def<int>(d, names::archiver_length, history_.size());
#endif
  }

  void nest::Archiving_Node::set_status(const DictionaryDatum & d)
  {
    // We need to preserve values in case invalid values are set
    double_t new_tau_minus = tau_minus_;
    double_t new_tau_minus_triplet = tau_minus_triplet_;
    updateValue<double_t>(d, names::tau_minus, new_tau_minus);
    updateValue<double_t>(d, names::tau_minus_triplet, new_tau_minus_triplet);

    if ( new_tau_minus <= 0 || new_tau_minus_triplet <= 0 )
      throw BadProperty("All time constants must be strictly positive.");

    tau_minus_ = new_tau_minus;
    tau_minus_triplet_ = new_tau_minus_triplet;

    // check, if to clear spike history and K_minus
    bool clear = false;
    updateValue<bool>(d, names::clear, clear);
    if ( clear )
	clear_history();
  }

  void nest::Archiving_Node::clear_history()
  {
      last_spike_ = -1.0;
      Kminus_ = 0.0;
      triplet_Kminus_ = 0.0;
      history_.clear();
  }

} // of namespace nest
