/*
 *  axonal_delay_archiving_node.h
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

#ifndef AXONAL_DELAY_ARCHIVING_NODE_H
#define AXONAL_DELAY_ARCHIVING_NODE_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "nest_time.h"
#include "nest_types.h"
#include "static_assert.h"
#include "structural_plasticity_node.h"

namespace nest
{

/**
 * Information on a received pre-synaptic spike which is buffered by an AxonalDelayArchivingNode, so that the weight of
 * the synapse which transmitted it, and the spike itself, can be corrected retrospectively.
 *
 * Entries are handed to the synapse when it is corrected; the synapse reads the entry and records the outcome of the
 * correction in it.
 */
class CorrectionEntrySTDPAxDelay
{
public:
  CorrectionEntrySTDPAxDelay( const size_t lcid,
    const synindex syn_id,
    const double t_last_pre_spike,
    const double weight_revert,
    const double new_weight,
    const double K_plus_revert,
    const long next_offset = -1 )
    : lcid_( lcid )
    , syn_id_( syn_id )
    , t_last_pre_spike_( t_last_pre_spike )
    , weight_revert_( weight_revert )
    , new_weight_( new_weight )
    , K_plus_revert_( K_plus_revert )
    , next_offset_( next_offset )
  {
  }

  //! Local connection index of the synapse which transmitted the spike.
  size_t
  get_lcid() const
  {
    return lcid_;
  }

  //! Synapse type of the synapse which transmitted the spike.
  synindex
  get_syn_id() const
  {
    return syn_id_;
  }

  //! Time of the last pre-synaptic spike before this spike.
  double
  get_t_last_pre_spike() const
  {
    return t_last_pre_spike_;
  }

  //! Synaptic weight to revert to (STDP depression needs to be undone) before a correction.
  double
  get_weight_revert() const
  {
    return weight_revert_;
  }

  void
  set_weight_revert( const double weight_revert )
  {
    weight_revert_ = weight_revert;
  }

  //! Weight of the synapse after the latest correction.
  double
  get_new_weight() const
  {
    return new_weight_;
  }

  void
  set_new_weight( const double new_weight )
  {
    new_weight_ = new_weight;
  }

  //! Pre-synaptic trace before the possibly incorrect facilitation.
  double
  get_K_plus_revert() const
  {
    return K_plus_revert_;
  }

  /**
   * Whether another spike of the same connection has been buffered after this one. If so, that spike has to revert to
   * the weight this entry ends up with.
   */
  bool
  has_next() const
  {
    return next_offset_ != -1;
  }

  /**
   * Steps from the slot of this entry to the slot of the next spike on this connection. Stored as an interval rather
   * than a lag because the lag of a given slot decreases by min_delay every slice.
   */
  long
  get_next_offset() const
  {
    return next_offset_;
  }

  void
  set_next_offset( const long next_offset )
  {
    next_offset_ = next_offset;
  }

private:
  unsigned int lcid_;        //!< local connection index
  unsigned int syn_id_;      //!< synapse-type index
  double t_last_pre_spike_;  //!< time of the last pre-synaptic spike before this spike
  double weight_revert_;     //!< synaptic weight to revert to (STDP depression needs to be undone)
  double new_weight_;        //!< new weight after the latest correction
  double K_plus_revert_;     //!< pre-synaptic trace before possibly incorrect facilitation
  long next_offset_;         //!< steps to the next spike on this connection (-1 if none)
};

//! check for correct correction entry size
using success_correction_entry_size = StaticAssert< sizeof( CorrectionEntrySTDPAxDelay ) == 48 >::success;

/**
 * A node which buffers the information needed to retrospectively correct incoming STDP synapses with
 * predominantly axonal delays.
 *
 * Due to the long axonal delays, relevant spikes of this neuron might not yet be available at the time when
 * incoming synapses are updated (spike delivery). Therefore, for each spike received through an STDP synapse
 * with predominantly axonal delay, information is stored for a short period of time allowing for
 * retrospective correction of the synapse and the already delivered spike.
 *
 * This is deliberately kept separate from ArchivingNode, which archives post-synaptic spike history: the two
 * concerns are independent, and a model can support axonal delays without inheriting the spike history (as
 * NESTML-generated models do, which derive from StructuralPlasticityNode and generate their own archiving
 * code).
 *
 * A model that derives from this class only supports being the target of a synapse with predominantly axonal
 * delay if it additionally
 *  - calls clear_expired_correction_entries_stdp_ax_delay_() for every lag in update(),
 *  - calls correct_synapses_stdp_ax_delay_() whenever it emits a spike, and accounts for the returned number
 *    of corrections in whatever spike archive it maintains,
 *  - handles WeightCorrectionEvent, and
 *  - overrides supports_axonal_delay_corrections() to return true.
 */
class AxonalDelayArchivingNode : public StructuralPlasticityNode
{
public:
  AxonalDelayArchivingNode();

  AxonalDelayArchivingNode( const AxonalDelayArchivingNode& );

  /**
   * Register a new incoming STDP connection, so this node knows whether it has to maintain correction
   * entries at all.
   *
   * Nodes which also archive post-synaptic spike history reach this through
   * ArchivingNode::register_stdp_connection(); nodes which do not have to call it themselves.
   */
  void register_axonal_delay_connection( const double dendritic_delay, const double axonal_delay );

  /**
   * Buffer a correction entry for the critical time window.
   *
   * @param spike_event Incoming pre-synaptic spike which could potentially need a correction after the next
   * post-synaptic spike.
   * @param t_last_pre_spike The time of the last pre-synaptic spike that was processed before the current one.
   * @param weight_revert The synaptic weight before depression after facilitation as baseline for potential later
   * correction.
   * @param new_weight The synaptic weight after the spike was transmitted, i.e., the weight the synapse is left with
   * if no correction follows.
   * @param K_plus_revert The pre-synaptic trace before the possibly incorrect facilitation.
   * @param time_while_critical The number of time steps until the spike no longer needs to be corrected.
   */
  void add_correction_entry_stdp_ax_delay( SpikeEvent& spike_event,
    const double t_last_pre_spike,
    const double weight_revert,
    const double new_weight,
    const double K_plus_revert,
    const double time_while_critical );

  /**
   * In case a correction is applied for a pre-synaptic spike, any other pre-synaptic spikes from the same synapse need
   * to be informed of the new base weight to revert to for the correction.
   */
  void update_weight_revert( const size_t lcid, const double weight_revert );

protected:
  //! Allocate or resize the correction buffer if the delay extrema changed since the last run. The buffer is only
  //! allocated for nodes with incoming connections with predominantly axonal delay.
  void pre_run_hook_();

  //! Retire the correction entries which are no longer critical in the given time slot.
  void clear_expired_correction_entries_stdp_ax_delay_( const size_t lag );

  /**
   * Triggered when this neuron spikes, to correct all relevant incoming STDP synapses with predominantly axonal delays
   * and corresponding received spikes.
   *
   * @returns the number of corrected synapses, i.e., the number of incoming STDP connections which have seen
   * this spike through a correction rather than through the regular spike history.
   */
  size_t correct_synapses_stdp_ax_delay_( const Time& t_spike );

private:
  /**
   * Buffer of correction entries sorted by t_spike_pre + delay (i.e., the actual arrival time at this neuron).
   */
  std::vector< std::vector< CorrectionEntrySTDPAxDelay > > correction_entries_stdp_ax_delay_;

  //! false by default and set to true if any incoming connection has predominant axonal delays
  bool has_predominant_stdp_ax_delay_;
};

}  // of namespace

#endif
