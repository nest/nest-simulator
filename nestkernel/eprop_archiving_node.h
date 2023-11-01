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
#include "archiving_node.h"
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "synaptic_element.h"

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

  void erase_unneeded_update_history();
  void erase_unneeded_eprop_history();
  void erase_unneeded_firing_rate_reg_history();

  void write_surrogate_gradient_to_history( long time_step, double surrogate_gradient );
  void write_error_signal_to_history( long time_step, double error_signal );
  void write_learning_signal_to_history( long& time_step, long& delay_out_rec, double& weight, double& error_signal );
  void write_update_to_history( long t_previous_update, long t_current_update );
  void write_firing_rate_reg_to_history( long t_current_update, double f_target, double c_reg );
  void write_eprop_parameter_to_map( std::string parameter_name, double parameter_value );

  const long get_shift() const;
  double get_firing_rate_reg( long time_step );
  void get_eprop_history( long time_step, std::deque< HistEntryEpropArchive >::iterator* it );
  std::map< std::string, double >& get_eprop_parameter_map();

  void add_spike_to_counter();
  void reset_spike_counter();

private:
  size_t n_spikes_ = 0;

  // These shifts are, for now, hardcoded to 1 time step / 1 ms since the current implementation only works if all the
  // delays are equal to the resolution of the simulation, i.e., 1 ms.

  long offset_gen = 1;     // offset since generator signals start from time step 1
  long delay_in_rec = 1;   // connection delay from input to recurrent neurons
  long delay_rec_out = 1;  // connection delay from recurrent to output neurons
  long delay_out_norm = 1; // connection delay between output neurons for normalization

  std::deque< HistEntryEpropArchive > eprop_history_;
  std::vector< HistEntryEpropFiringRateReg > firing_rate_reg_history_;
  std::vector< HistEntryEpropUpdate > update_history_;

  std::map< std::string, double > eprop_parameter_map_;
};

} // namespace nest
#endif // EPROP_ARCHIVING_NODE_H
