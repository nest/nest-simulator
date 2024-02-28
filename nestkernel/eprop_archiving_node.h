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
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

// sli
#include "dictdatum.h"

namespace nest
{

/**
 * Base class implementing an intermediate archiving node model for node models supporting e-prop plasticity.
 *
 * A node which archives the history of dynamic variables, the firing rate
 * regularization, and update times needed to calculate the weight updates for
 * e-prop plasticity. It further provides a set of get, write, and set functions
 * for these histories and the hardcoded shifts to synchronize the factors of
 * the plasticity rule.
 */
template < typename HistEntryT >
class EpropArchivingNode : public Node
{

public:
  //! Default constructor.
  EpropArchivingNode();

  //! Copy constructor.
  EpropArchivingNode( const EpropArchivingNode& );

  //! Initialize the update history and register the eprop synapse.
  void register_eprop_connection() override;

  //! Register current update in the update history and deregister previous update.
  void write_update_to_history( const long t_previous_update, const long t_current_update ) override;

  //! Get an iterator pointing to the update history entry of the given time step.
  std::vector< HistEntryEpropUpdate >::iterator get_update_history( const long time_step );

  //! Get an iterator pointing to the eprop history entry of the given time step.
  typename std::vector< HistEntryT >::iterator get_eprop_history( const long time_step );

  //! Erase update history parts for which the access counter has decreased to zero since no synapse needs them
  //! any longer.
  void erase_used_update_history();

  //! Erase update intervals from the e-prop history in which each synapse has either not transmitted a spike or has
  //! transmitted a spike in a more recent update interval.
  void erase_used_eprop_history();

protected:
  //!< Number of incoming eprop synapses
  size_t eprop_indegree_;

  //! History of updates still needed by at least one synapse.
  std::vector< HistEntryEpropUpdate > update_history_;

  //! History of dynamic variables needed for e-prop plasticity.
  std::vector< HistEntryT > eprop_history_;

  // The following shifts are, for now, hardcoded to 1 time step since the current
  // implementation only works if all the delays are equal to the simulation resolution.

  //! Offset since generator signals start from time step 1.
  const long offset_gen_ = 1;

  //! Connection delay from input to recurrent neurons.
  const long delay_in_rec_ = 1;

  //! Connection delay from recurrent to output neurons.
  const long delay_rec_out_ = 1;

  //! Connection delay between output neurons for normalization.
  const long delay_out_norm_ = 1;

  //! Connection delay from output neurons to recurrent neurons.
  const long delay_out_rec_ = 1;
};

/**
 * Class implementing an intermediate archiving node model for recurrent node models supporting e-prop plasticity.
 */
class EpropArchivingNodeRecurrent : public EpropArchivingNode< HistEntryEpropRecurrent >
{

public:
  //! Default constructor.
  EpropArchivingNodeRecurrent();

  //! Copy constructor.
  EpropArchivingNodeRecurrent( const EpropArchivingNodeRecurrent& );

  //! Create an entry in the eprop history for the given time step and surrogate gradient.
  void write_surrogate_gradient_to_history( const long time_step, const double surrogate_gradient );

  //! Update the learning signal in the eprop history entry of the given time step by writing the value of the incoming
  //! learning signal to the history or adding it to the existing value in case of multiple readout neurons.
  void write_learning_signal_to_history( const long time_step, const double learning_signal );

  //! Create an entry in the firing rate regularization history for the current update.
  void write_firing_rate_reg_to_history( const long t_current_update, const double f_target, const double c_reg );

  //! Get an iterator pointing to the firing rate regularization history of the given time step.
  std::vector< HistEntryEpropFiringRateReg >::iterator get_firing_rate_reg_history( const long time_step );

  //! Return learning signal from history for given time step or zero if time step not in history
  double get_learning_signal_from_history( const long time_step );

  //! Erase parts of the firing rate regularization history for which the access counter in the update history has
  //! decreased to zero since no synapse needs them any longer.
  void erase_used_firing_rate_reg_history();

  //! Count emitted spike for the firing rate regularization.
  void count_spike();

  //! Reset spike count for the firing rate regularization.
  void reset_spike_count();

private:
  //! Count of the emitted spikes for the firing rate regularization.
  size_t n_spikes_;

  //! History of the firing rate regularization.
  std::vector< HistEntryEpropFiringRateReg > firing_rate_reg_history_;
};

inline void
EpropArchivingNodeRecurrent::count_spike()
{
  ++n_spikes_;
}

inline void
EpropArchivingNodeRecurrent::reset_spike_count()
{
  n_spikes_ = 0;
}

/**
 * Class implementing an intermediate archiving node model for readout node models supporting e-prop plasticity.
 */
class EpropArchivingNodeReadout : public EpropArchivingNode< HistEntryEpropReadout >
{
public:
  //! Default constructor.
  EpropArchivingNodeReadout();

  //! Copy constructor.
  EpropArchivingNodeReadout( const EpropArchivingNodeReadout& );

  //! Create an entry in the eprop history for the given time step and error signal.
  void write_error_signal_to_history( const long time_step, const double error_signal );
};

} // namespace nest

#endif // EPROP_ARCHIVING_NODE_H
