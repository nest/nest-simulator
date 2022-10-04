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

#ifndef ARCHIVING_NODE_H
#define ARCHIVING_NODE_H

// C++ includes:
#include <algorithm>
#include <deque>

// Includes from nestkernel:
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"
#include "structural_plasticity_node.h"

// Includes from sli:
#include "dictdatum.h"

#define DEBUG_ARCHIVER 1

namespace nest
{

/**
 * A node which archives spike history for the purposes of spike-timing
 * dependent plasticity (STDP)
 */
class ArchivingNode : public StructuralPlasticityNode
{
public:
  /**
   * \fn ArchivingNode()
   * Constructor.
   */
  ArchivingNode();

  /**
   * \fn ArchivingNode()
   * Copy Constructor.
   */
  ArchivingNode( const ArchivingNode& );

  /**
   * \fn double get_K_value(long t)
   * return the Kminus (synaptic trace) value at t (in ms). When the trace is
   * requested at the exact same time that the neuron emits a spike, the trace
   * value as it was just before the spike is returned.
   */
  double get_K_value( double t );

  /**
   * \fn void get_K_values( double t,
   *   double& Kminus,
   *   double& nearest_neighbor_Kminus,
   *   double& Kminus_triplet )
   * write the Kminus (eligibility trace for STDP),
   * nearest_neighbour_Kminus (eligibility trace for nearest-neighbour STDP:
   *   like Kminus, but increased to 1, rather than by 1, on a spike
   *   occurrence),
   * and Kminus_triplet
   * values at t (in ms) to the provided locations.
   * @throws UnexpectedEvent
   */
  void get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet );

  /**
   * \fn void get_K_values( double t,
   *   double& Kminus,
   *   double& Kminus_triplet )
   * The legacy version of the function, kept for compatibility
   * after changing the function signature in PR #865.
   * @throws UnexpectedEvent
   */
  void
  get_K_values( double t, double& Kminus, double& Kminus_triplet )
  {
    double nearest_neighbor_Kminus_to_discard;
    get_K_values( t, Kminus, nearest_neighbor_Kminus_to_discard, Kminus_triplet );
  }

  /**
   * \fn double get_K_triplet_value(std::deque<histentry>::iterator &iter)
   * return the triplet Kminus value for the associated iterator.
   */
  double get_K_triplet_value( const std::deque< histentry >::iterator& iter );

  /**
   * \fn void get_history(long t1, long t2,
   * std::deque<Archiver::histentry>::iterator* start,
   * std::deque<Archiver::histentry>::iterator* finish)
   * return the spike times (in steps) of spikes which occurred in the range
   * (t1,t2].
   */
  void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish );

  /**
   * Register a new incoming STDP connection.
   *
   * t_first_read: The newly registered synapse will read the history entries
   * with t > t_first_read.
   */
  void register_stdp_connection( double t_first_read, double delay );

  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d );

protected:
  /**
   * \fn void set_spiketime(Time const & t_sp, double offset)
   * record spike history
   */
  void set_spiketime( Time const& t_sp, double offset = 0.0 );

  /**
   * \fn double get_spiketime()
   * return most recent spike time in ms
   */
  inline double get_spiketime_ms() const;

  /**
   * \fn void clear_history()
   * clear spike history
   */
  void clear_history();

  // number of incoming connections from stdp connectors.
  // needed to determine, if every incoming connection has
  // read the spikehistory for a given point in time
  size_t n_incoming_;

private:
  // sum exp(-(t-ti)/tau_minus)
  double Kminus_;

  // sum exp(-(t-ti)/tau_minus_triplet)
  double Kminus_triplet_;

  double tau_minus_;
  double tau_minus_inv_;

  // time constant for triplet low pass filtering of "post" spike train
  double tau_minus_triplet_;
  double tau_minus_triplet_inv_;

  double max_delay_;
  double trace_;

  double last_spike_;

  // spiking history needed by stdp synapses
  std::deque< histentry > history_;
};

inline double
ArchivingNode::get_spiketime_ms() const
{
  return last_spike_;
}

} // of namespace
#endif
