/*
 *  paced_spiking_mechanism.h
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

#ifndef PACED_SPIKING_MECHANISM_H
#define PACED_SPIKING_MECHANISM_H

// nestkernel
#include "nest_time.h"

// libnestutil
#include "dict_util.h"

namespace nest
{
/**
 * @brief Class implementing a paced spiking mechanism for neuron models.
 *
 * This class implements a paced spiking mechanism.
 */
class PacedSpikingMechanism
{
public:
  /**
   * Default constructor.
   */
  PacedSpikingMechanism();

  /**
   * Copy constructor.
   *
   * @param n The other object to copy.
   */
  PacedSpikingMechanism( const PacedSpikingMechanism& n );

  /**
   * Virtual destructor.
   */
  virtual ~PacedSpikingMechanism() = default;

  /**
   * Re-calculates dependent parameters.
   */
  void pre_run_hook();

  /**
   * Checks if a paced spiking is due at the current time.
   */
  inline bool
  emit_spike( bool emit_dynamic_spike )
  {
    if ( not paced_spiking_ )
    {
      return emit_dynamic_spike;
    }

    --steps_until_paced_spike_;
    const bool emit_paced_spike = steps_until_paced_spike_ == 0;
    if ( emit_paced_spike )
    {
      steps_until_paced_spike_ = paced_spiking_interval_steps_;
    }

    return emit_paced_spike;
  }

  /**
   * Retrieves parameters and adds them to the status dictionary.
   */
  void get_status( Dictionary& d ) const;

  /**
   * Sets and validates parameters from the status dictionary.
   */
  void set_status( const Dictionary& d );

protected:
  //! If True, the neuron is forced to spike in set intervals.
  bool paced_spiking_;

  //! Temporal offset of paced spiking (ms).
  double paced_spiking_offset_;

  //! Interval between two consecutive paced spikes (ms).
  double paced_spiking_interval_;

  //! Interval steps between two consecutive paced spikes.
  long paced_spiking_interval_steps_;

  //! Remaining steps until the next paced spike.
  long steps_until_paced_spike_;
};

}  // namespace nest

#endif  // PACED_SPIKING_MECHANISM_H
