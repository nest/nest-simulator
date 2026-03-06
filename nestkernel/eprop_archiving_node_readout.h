/*
 *  eprop_archiving_node_readout.h
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

#ifndef EPROP_ARCHIVING_NODE_READOUT_H
#define EPROP_ARCHIVING_NODE_READOUT_H

// models
#include "eprop_archiving_node_impl.h"

// nestkernel
#include "histentry.h"

namespace nest
{

/**
 * Class implementing an intermediate archiving node model for readout node models supporting e-prop plasticity.
 */
template < bool hist_shift_required >
class EpropArchivingNodeReadout : public EpropArchivingNode< HistEntryEpropReadout >
{
public:
  /**
   * Constructs a new EpropArchivingNodeReadout object.
   */
  EpropArchivingNodeReadout()
    : EpropArchivingNode()
  {
  }

  /**
   * Constructs a new EpropArchivingNodeReadout object by copying another EpropArchivingNodeReadout object.
   *
   * @param other The EpropArchivingNodeReadout object to copy.
   */
  EpropArchivingNodeReadout( const EpropArchivingNodeReadout& other )
    : EpropArchivingNode( other )
  {
  }

  /**
   * Creates an entry for the specified time step at the end of the eprop history.
   *
   * @param time_step The time step.
   */
  void append_new_eprop_history_entry( long time_step );

  /**
   * Writes the error signal to the eprop history at the specified time step.
   *
   * @param time_step The time step.
   * @param error_signal The error signal.
   */
  void write_error_signal_to_history( long time_step, const double error_signal );

protected:
  long model_dependent_history_shift_() const override;
  bool history_shift_required_() const override;
};
}

#endif
