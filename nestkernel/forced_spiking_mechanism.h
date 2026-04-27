/*
 *  forced_spiking_mechanism.h
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

#ifndef FORCED_SPIKING_MECHANISM_H
#define FORCED_SPIKING_MECHANISM_H

// nestkernel
#include "nest_time.h"

// libnestutil
#include "dict_util.h"

namespace nest
{
/**
 * @brief Class implementing a forced spiking mechanism for neuron models.
 *
 * This class implements a forced spiking mechanism.
 */
class ForcedSpikingMechanism
{
public:
  /**
   * Default constructor.
   */
  ForcedSpikingMechanism();

  /**
   * Copy constructor.
   *
   * @param n The other object to copy.
   */
  ForcedSpikingMechanism( const ForcedSpikingMechanism& n );

  /**
   * Virtual destructor.
   */
  virtual ~ForcedSpikingMechanism() = default;

  /**
   * Re-calculates dependent parameters.
   */
  void pre_run_hook();

  /**
   * Checks if a forced spiking is due at the current time.
   */
  inline bool
  emit_spike( bool emit_dynamic_spike )
  {
    if ( not forced_spiking_ )
    {
      return emit_dynamic_spike;
    }

    const bool emit_forced_spike = steps_until_forced_spike_ % forced_spiking_interval_steps_ == 0;
    ++steps_until_forced_spike_;
    return emit_forced_spike;
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
  bool forced_spiking_;

  //! Temporal offset of forced spiking (ms).
  double forced_spiking_offset_;

  //! Interval between two consecutive forced spikes (ms).
  double forced_spiking_interval_;

  //! Interval steps between two consecutive forced spikes.
  long forced_spiking_interval_steps_;

  //! Remaining steps until the next forced spike.
  long steps_until_forced_spike_;
};

}  // namespace nest

#endif  // FORCED_SPIKING_MECHANISM_H
