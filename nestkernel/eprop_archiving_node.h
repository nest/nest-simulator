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

class EpropArchivingNode : public ArchivingNode
{

public:
  EpropArchivingNode();
  EpropArchivingNode( const EpropArchivingNode& );

  void init_update_history( double delay );
  double get_firing_rate_reg( double time_point );

protected:
  void erase_unneeded_update_history();
  void erase_unneeded_eprop_history();
  void erase_unneeded_firing_rate_reg_history();

  void write_v_m_pseudo_deriv_to_history( long time_step, double v_m_pseudo_deriv );
  void write_error_signal_to_history( long time_step, double error_signal );
  void write_learning_signal_to_history( LearningSignalConnectionEvent& e );
  void write_update_to_history( double t_last_update, double t_current_update );
  void write_firing_rate_reg_to_history( double t_current_update, double f_target, double c_reg );

  void get_eprop_history( double time_point, std::deque< histentry_eprop_archive >::iterator* it );

  void add_spike_to_counter();
  void reset_spike_counter();


private:
  double eps_ = 1e-6;
  size_t n_spikes_ = 0;

  std::deque< histentry_eprop_archive > eprop_history_;
  std::vector< histentry_eprop_firing_rate_reg > firing_rate_reg_history_;
  std::vector< histentry_eprop_update > update_history_;
};

} // namespace nest
#endif // EPROP_ARCHIVING_NODE_H
