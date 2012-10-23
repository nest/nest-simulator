/*
 *  archiving_node.h
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
 * \file archiving_node.h
 * Definition of Archiving_Node which is capable of
 * recording and managing a spike history. 
 * \author Moritz Helias, Abigail Morrison
 * \date april 2006
 */

#ifndef ARCHIVING_NODE_H
#define ARCHIVING_NODE_H

#include "nest.h"
#include "node.h"
#include "dictdatum.h"
#include "nest_time.h"
#include "histentry.h"
#include <deque>

#define DEBUG_ARCHIVER 1

namespace nest {

/**
 * \class Archiving_Node
 * a node which archives spike history for the purposes of
 * timing dependent plasticity
 */
  class Archiving_Node: 
    public Node 
{
   
 public:
  

  /**
   * \fn Archiving_Node()
   * Constructor.
   */
  Archiving_Node();

  /**
   * \fn Archiving_Node()
   * Copy Constructor.
   */
  Archiving_Node(const Archiving_Node&);


  /**
   * \fn double_t get_K_value(long_t t)
   * return the Kminus value at t (in ms).
   */
  double_t get_K_value(double_t t);

  /**
   * write the Kminus and triplet_Kminus values at t (in ms) to
   * the provided locations.
   * @throws UnexpectedEvent
   */

  void get_K_values(double_t t, double_t& Kminus, double_t& triplet_Kminus); 

  /**
   * \fn double_t get_triplet_K_value(std::deque<histentry>::iterator &iter)
   * return the triplet Kminus value for the associated iterator.
   */

  double_t get_triplet_K_value(const std::deque<histentry>::iterator &iter);
  
  /**
   * \fn void get_history(long_t t1, long_t t2, std::deque<Archiver::histentry>::iterator* start, std::deque<Archiver::histentry>::iterator* finish)
   * return the spike times (in steps) of spikes which occurred in the range (t1,t2].
   */
  void get_history(double_t t1, double_t t2, 
                   std::deque<histentry>::iterator* start,
  		   std::deque<histentry>::iterator* finish);

  /**
   * Register a new incoming STDP connection.
   * 
   * t_first_read: The newly registered synapse will read the history entries with t > t_first_read.
   */
  void register_stdp_connection(double_t t_first_read);

  /**
   * Unregister this incoming connection.
   * t_last_read: The unregistered synapse has read all history entries with t <= t_last_read.
   */
  void unregister_stdp_connection(double_t t_last_read);


  void get_status(DictionaryDatum & d) const;
  void set_status(const DictionaryDatum & d);

 protected:

  /**
   * \fn void set_spiketime(Time const & t_sp)
   * record spike history
   */
  void set_spiketime(Time const & t_sp);

  /**
   * \fn double_t get_spiketime()
   * return most recent spike time in ms
   */ 
  inline double_t get_spiketime_ms() const;

  /**
   * \fn void clear_history()
   * clear spike history
   */
  void clear_history();


 private:

  // number of incoming connections from stdp connectors.
  // needed to determine, if every incoming connection has
  // read the spikehistory for a given point in time
  size_t n_incoming_;

  // sum exp(-(t-ti)/tau_minus)
  double_t Kminus_;

  // sum exp(-(t-ti)/tau_minus_triplet)
  double_t triplet_Kminus_;

  double_t tau_minus_;

  // time constant for triplet low pass filtering of "post" spike train
  double_t tau_minus_triplet_;

  double_t last_spike_;

  // spiking history needed by stdp synapses
  std::deque<histentry> history_;

};
  
inline 
double_t Archiving_Node::get_spiketime_ms() const
{
   return last_spike_;
}

} // of namespace

#endif



