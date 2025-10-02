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
  ArchivingNode();

  ArchivingNode( const ArchivingNode& );

  /**
   * Return the Kminus (synaptic trace) value at time t (given in ms).
   *
   * When the trace is requested at the exact same time that the neuron emits a spike,
   * the trace value as it was just before the spike is returned.
   */
  double get_K_value( double t ) override;

  /**
   * Write the different STDP K values at time t (in ms) to the provided locations.
   *
   * @param Kminus the eligibility trace for STDP
   * @param nearest_neighbour_Kminus eligibility trace for nearest-neighbour STDP, like Kminus,
                                     but increased to 1, rather than by 1, when a spike occurs
   * @param Kminus_triplet eligibility trace for triplet STDP
   *
   * @throws UnexpectedEvent
   */
  void get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet ) override;

  /**
   * Return the triplet Kminus value for the associated iterator.
   */
  double get_K_triplet_value( const std::deque< histentry >::iterator& iter );

  /**
   * Return the spike times (in steps) of spikes which occurred in the range [t1,t2].
   */
  void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish ) override;

  /**
   * Register a new incoming STDP connection.
   *
   * t_first_read: The newly registered synapse will read the history entries
   * with t > t_first_read.
   */
  void register_stdp_connection( double t_first_read, double delay ) override;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

protected:
  /**
   * Record spike history
   */
  void set_spiketime( Time const& t_sp, double offset = 0.0 );

  /**
   * Return most recent spike time in ms
   */
  double get_spiketime_ms() const;

  /**
   * Clear spike history
   */
  void clear_history();

  /**
   * Number of incoming connections from STDP connectors.
   *
   * This variable is needed to determine if every incoming connection has
   * read the spikehistory for a given point in time
   */
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

} // of namespace
#endif
