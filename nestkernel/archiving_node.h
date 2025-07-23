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
  void register_stdp_connection( const double t_first_read,
    const double dendritic_delay,
    const double axonal_delay ) override;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

  /**
   * Framework for STDP with predominantly axonal delays: Buffer a correction entry for a short time window.
   *
   * @param spike_event Incoming pre-synaptic spike which could potentially need a correction after the next
   * post-synaptic spike.
   * @param t_last_pre_spike The time of the last pre-synaptic spike that was processed before the current one.
   * @param weight_revert The synaptic weight before depression after facilitation as baseline for potential later
   * correction.
   * @param time_while_critical The number of time steps until the spike no longer needs to be corrected.
   */
  void add_correction_entry_stdp_ax_delay( SpikeEvent& spike_event,
    const double t_last_pre_spike,
    const double weight_revert,
    const double K_plus_revert,
    const double time_while_critical );

protected:
  void pre_run_hook_();

  /**
   * Record spike history
   */
  void set_spiketime( Time const& t_sp, double offset = 0.0 );

  /**
   * Return most recent spike time in ms
   */
  inline double get_spiketime_ms() const;

  /**
   * Clear spike history
   */
  void clear_history();

  void reset_correction_entries_stdp_ax_delay_();

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

  /**
   * Framework for STDP with predominantly axonal delays:
   * Due to the long axonal delays, relevant spikes of this neuron might not yet be available at the time when incoming
   * synapses are updated (spike delivery). Therefore, for each spike received through an STDP synapse with
   * predominantly axonal delay, information is stored for a short period of time allowing for retrospective correction
   * of the synapse and the already delivered spike.
   */
  struct CorrectionEntrySTDPAxDelay
  {
    CorrectionEntrySTDPAxDelay( const size_t lcid,
      const synindex syn_id,
      const double t_last_pre_spike,
      const double weight_revert,
      const double K_plus_revert )
      : lcid_( lcid )
      , syn_id_( syn_id )
      , t_last_pre_spike_( t_last_pre_spike )
      , weight_revert_( weight_revert )
      , K_plus_revert_( K_plus_revert )
    {
    }

    unsigned int lcid_;       //!< local connection index
    synindex syn_id_;         //!< synapse-type index
    double t_last_pre_spike_; //!< time of the last pre-synaptic spike before this spike
    double weight_revert_;    //!< synaptic weight to revert to (STDP depression needs to be undone)
    double K_plus_revert_;    //!< pre-synaptic trace before possibly incorrect facilitation
  };

  //! check for correct correction entry size
  using correction_entry_size = StaticAssert< sizeof( ArchivingNode::CorrectionEntrySTDPAxDelay ) == 32 >::success;

protected:
  /**
   * Framework for STDP with predominantly axonal delays:
   * Buffer of correction entries sorted by t_spike_pre + delay
   * (i.e., the actual arrival time at this neuron).
   */
  std::vector< std::vector< CorrectionEntrySTDPAxDelay > > correction_entries_stdp_ax_delay_;
  //! false by default and set to true if any incoming connection has predominant axonal delays
  bool has_predominant_stdp_ax_delay_;

  /**
   * Framework for STDP with predominantly axonal delays:
   * Triggered when this neuron spikes, to correct all relevant incoming STDP synapses with predominantly axonal delays
   * and corresponding received spikes. */
  void correct_synapses_stdp_ax_delay_( const Time& t_spike );
};

inline double
ArchivingNode::get_spiketime_ms() const
{
  return last_spike_;
}

} // of namespace
#endif
