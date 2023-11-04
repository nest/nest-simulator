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

// C++
#include <deque>

// nestkernel
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

// sli
#include "dictdatum.h"

/**
 * A node which archives the spike history and the history of the
 * dynamic variables associated with e-prop plasticity.
 */
namespace nest
{

class EpropArchivingNode : public Node
{

public:
  EpropArchivingNode();
  EpropArchivingNode( const EpropArchivingNode& );

  void init_update_history();

  void write_update_to_history( long t_previous_update, long t_current_update );
  void write_surrogate_gradient_to_history( long time_step, double surrogate_gradient );
  void write_error_signal_to_history( long time_step, double error_signal );
  void write_learning_signal_to_history( long& time_step, long& delay_out_rec, double& weight, double& error_signal );
  void write_firing_rate_reg_to_history( long t_current_update, double f_target, double c_reg );

  const long get_shift() const;
  const long get_delay_out_norm() const;
  std::deque< HistEntryEpropArchive >::iterator get_eprop_history( long time_step );
  double get_firing_rate_reg( long time_step );

  void erase_unneeded_update_history();
  void erase_unneeded_eprop_history();
  void erase_unneeded_firing_rate_reg_history();

  void count_spike();
  void reset_spike_count();

private:
  size_t n_spikes_ = 0;

  // These shifts are, for now, hardcoded to 1 time step since the current implementation only works if all the
  // delays are equal to the resolution of the simulation.

  long offset_gen_ = 1;     //!< offset since generator signals start from time step 1
  long delay_in_rec_ = 1;   //!< connection delay from input to recurrent neurons
  long delay_rec_out_ = 1;  //!< connection delay from recurrent to output neurons
  long delay_out_norm_ = 1; //!< connection delay between output neurons for normalization

  std::vector< HistEntryEpropFiringRateReg > firing_rate_reg_history_;
  std::vector< HistEntryEpropUpdate > update_history_;

protected:
  std::deque< HistEntryEpropArchive > eprop_history_;
};

} // namespace nest
#endif // EPROP_ARCHIVING_NODE_H
