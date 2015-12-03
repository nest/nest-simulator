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
#include "synaptic_element.h"

#define DEBUG_ARCHIVER 1

namespace nest
{

/**
 * \class Archiving_Node
 * a node which archives spike history for the purposes of
 * timing dependent plasticity
 */
class Archiving_Node : public Node
{
  using Node::get_synaptic_elements;

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
  Archiving_Node( const Archiving_Node& );

  /**
   * \fn double_t get_Ca_minus()
   * return the current value of Ca_minus
   */
  double_t get_Ca_minus() const;

  /**
   * \fn double_t get_synaptic_elements(Name n)
   * get the number of synaptic element for the current Node
   * the number of synaptic elements is a double value but the number of
   * actual vacant and connected elements is an integer truncated from this
   * value
   */
  double_t get_synaptic_elements( Name n ) const;

  /**
   * \fn int_t get_synaptic_elements_vacant(Name n)
   * get the number of synaptic elements of type n which are available
   * for new synapse creation
   */
  int_t get_synaptic_elements_vacant( Name n ) const;

  /**
   * \fn int_t get_synaptic_elements_connected(Name n)
   * get the number of synaptic element of type n which are currently
   * connected
   */
  int_t get_synaptic_elements_connected( Name n ) const;

  /**
   * \fn std::map<Name, double_t> get_synaptic_elements()
   * get the number of all synaptic elements for the current Node
   */
  std::map< Name, double_t > get_synaptic_elements() const;

  /**
   * \fn void update_synaptic_elements()
   * Change the number of synaptic elements in the node depending on the
   * dynamics described by the corresponding growth curve
   */
  void update_synaptic_elements( double_t t );

  /**
   * \fn void decay_synaptic_elements_vacant()
   * Delete a certain portion of the vacant synaptic elements which are not
   * in use
   */
  void decay_synaptic_elements_vacant( double_t p );

  /**
   * \fn void connect_synaptic_element()
   * Change the number of connected synaptic elements by n
   */
  void connect_synaptic_element( Name name, int_t n );

  /**
   * \fn double_t get_K_value(long_t t)
   * return the Kminus value at t (in ms).
   */
  double_t get_K_value( double_t t );

  /**
   * write the Kminus and triplet_Kminus values at t (in ms) to
   * the provided locations.
   * @throws UnexpectedEvent
   */

  void get_K_values( double_t t, double_t& Kminus, double_t& triplet_Kminus );

  /**
   * \fn double_t get_triplet_K_value(std::deque<histentry>::iterator &iter)
   * return the triplet Kminus value for the associated iterator.
   */

  double_t get_triplet_K_value( const std::deque< histentry >::iterator& iter );

  /**
   * \fn void get_history(long_t t1, long_t t2, std::deque<Archiver::histentry>::iterator* start,
   * std::deque<Archiver::histentry>::iterator* finish)
   * return the spike times (in steps) of spikes which occurred in the range (t1,t2].
   */
  void get_history( double_t t1,
    double_t t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish );

  /**
   * Register a new incoming STDP connection.
   *
   * t_first_read: The newly registered synapse will read the history entries with t > t_first_read.
   */
  void register_stdp_connection( double_t t_first_read );

  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d );

  /**
   * retrieve the current value of tau_Ca which defines the exponential decay
   * constant of the intracellular calcium concentration
   */
  double_t get_tau_Ca() const;

protected:
  /**
   * \fn void set_spiketime(Time const & t_sp)
   * record spike history
   */
  void set_spiketime( Time const& t_sp );

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
  std::deque< histentry > history_;

  /*
   * Structural plasticity
   */

  // Time of the last update of the Calcium concentration in ms
  double_t Ca_t_;

  // Value of the calcium concentration [Ca2+] at Ca_t_. Intracellular calcium
  // concentration has a linear factor to mean electrical activity of 10^2,
  // this means, for example, that a [Ca2+] of 0.2 is equivalent to a mean activity
  // of 20Hz.
  double_t Ca_minus_;

  // Time constant for exponential decay of the intracellular calcium concentration
  double_t tau_Ca_;

  // Increase in calcium concentration [Ca2+] for each spike of the neuron
  double_t beta_Ca_;

  // Map of the synaptic elements
  std::map< Name, SynapticElement > synaptic_elements_map_;
};

inline double_t
Archiving_Node::get_spiketime_ms() const
{
  return last_spike_;
}

inline double_t
Archiving_Node::get_tau_Ca() const
{
  return tau_Ca_;
}

inline double_t
Archiving_Node::get_Ca_minus() const
{
  return Ca_minus_;
}

} // of namespace
#endif
