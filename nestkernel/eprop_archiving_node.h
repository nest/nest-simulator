/*
 *  eprop_archiving_node.h
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

#ifndef EPROP_ARCHIVING_NODE_H
#define EPROP_ARCHIVING_NODE_H

// nestkernel
#include "histentry.h"
#include "node.h"

// sli
#include "dictdatum.h"

namespace nest
{
/**
 * @brief Base class implementing archiving for node models supporting e-prop plasticity.
 *
 * Base class implementing an intermediate archiving node model for node models supporting e-prop plasticity
 * according to Bellec et al. (2020) and supporting additional biological features described in Korcsak-Gorzo,
 * Stapmanns, and Espinoza Valverde et al. (in preparation).
 *
 * A node which archives the history of dynamic variables, the firing rate
 * regularization, and update times needed to calculate the weight updates for
 * e-prop plasticity. It further provides a set of get, write, and set functions
 * for these histories and the hardcoded shifts to synchronize the factors of
 * the plasticity rule.
 *
 * @tparam HistEntryT The type of history entry.
 */
template < typename HistEntryT >
class EpropArchivingNode : public Node
{

public:
  /**
   * Constructs a new EpropArchivingNode object.
   */
  EpropArchivingNode()
    : eprop_indegree_( 0 )
  {
  }

  /**
   * Constructs a new EpropArchivingNode object by copying another EpropArchivingNode object.
   *
   * @param other The other object to copy.
   */
  EpropArchivingNode( const EpropArchivingNode& other )
    : Node( other )
    , eprop_indegree_( other.eprop_indegree_ )
  {
  }

  void register_eprop_connection() override;

  void write_update_to_history( const long t_previous_update,
    const long t_current_update,
    const long eprop_isi_trace_cutoff = 0 ) override;

  /**
   * Retrieves the update history entry for a specific time step.
   *
   * @param time_step The time step.
   * @return An iterator pointing to the update history for the specified time step.
   */
  std::vector< HistEntryEpropUpdate >::iterator get_update_history( const long time_step );

  /**
   * Retrieves the eprop history entry for a specified time step.
   *
   * @param time_step The time step.
   * @return An iterator pointing to the eprop history entry for the specified time step.
   */
  typename std::vector< HistEntryT >::iterator get_eprop_history( const long time_step );

  /**
   * @brief Erases the used eprop history for `bsshslm_2020` models.
   *
   * Erases e-prop history entries for update intervals during which no spikes were sent to the target neuron,
   * and any entries older than the earliest time stamp required by the first update in the history.
   */
  void erase_used_eprop_history();

  /**
   * @brief Erases the used eprop history.
   *
   * Erases e-prop history entries between the last and penultimate updates if they exceed the inter-spike
   * interval trace cutoff and any entries older than the earliest time stamp required by the first update.
   *
   * @param eprop_isi_trace_cutoff The cutoff value for the inter-spike integration of the eprop trace.
   */
  void erase_used_eprop_history( const long eprop_isi_trace_cutoff );

  /**
   * @brief Retrieves eprop history size.
   *
   * Retrieves the size of the eprop history buffer.
   */
  double get_eprop_history_duration() const;

protected:
  //! Returns correct shift for history depending on whether it is a normal or a bsshslm_2020 model.
  virtual long model_dependent_history_shift_() const = 0;

  /**
   * Provides access to the template flag from the base class.
   *
   * @todo This should be removed once we revise the class structure.
   */
  virtual bool history_shift_required_() const = 0;

  //! Number of incoming eprop synapses
  size_t eprop_indegree_;

  //! History of updates still needed by at least one synapse.
  std::vector< HistEntryEpropUpdate > update_history_;

  //! History of dynamic variables needed for e-prop plasticity.
  std::vector< HistEntryT > eprop_history_;

  // The following shifts are, for now, hardcoded to 1 time step since the current
  // implementation only works if all the delays are equal to the simulation resolution.

  //! Offset since generator signals start from time step 1.
  const long offset_gen_ = 1;

  //! Transmission delay from input to recurrent neurons.
  const long delay_in_rec_ = 1;

  //! Transmission delay from recurrent to output neurons.
  const long delay_rec_out_ = 1;

  //! Transmission delay between output neurons for normalization.
  const long delay_out_norm_ = 1;

  //! Transmission delay from output neurons to recurrent neurons.
  const long delay_out_rec_ = 1;
};

} // namespace nest

#endif // EPROP_ARCHIVING_NODE_H
