/*
 *  ignore_and_spike_mechanism.h
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

#ifndef IGNORE_AND_SPIKE_MECHANISM_H
#define IGNORE_AND_SPIKE_MECHANISM_H

// nestkernel
#include "nest_time.h"

// libnestutil
#include "dict_util.h"

namespace nest
{
/**
 * @brief Class implementing an ignore-and-fire mechanism for neuron models.
 */
class IgnoreAndSpikeMechanism
{
public:
  /**
   * Default constructor.
   */
  IgnoreAndSpikeMechanism();

  /**
   * Copy constructor.
   *
   * @param n The other object to copy.
   */
  IgnoreAndSpikeMechanism( const IgnoreAndSpikeMechanism& n );

  /**
   * Virtual destructor.
   */
  virtual ~IgnoreAndSpikeMechanism() = default;

  /**
   * Re-calculates dependent parameters.
   */
  void pre_run_hook();

  /**
   * Updates spike schedule and returns whether a spike should be emitted.
   */
  inline bool
  update_and_check_spike_emission( bool emit_dynamic_spike )
  {
    if ( not ignore_and_spike_ )
    {
      return emit_dynamic_spike;
    }

    --ignore_and_spike_steps_until_spike_;
    const bool emit_ignore_and_spike = ignore_and_spike_steps_until_spike_ == 0;
    if ( emit_ignore_and_spike )
    {
      ignore_and_spike_steps_until_spike_ = ignore_and_spike_interval_steps_;
    }

    return emit_ignore_and_spike;
  }

  /**
   * Retrieves parameters and adds them to the status dictionary.
   */
  void get_status( Dictionary& d ) const;

  /**
   * Sets and validates parameters from the status dictionary.
   */
  void set_status( const Dictionary& d );

private:
  //! If True, the neuron is forced to spike in set intervals.
  bool ignore_and_spike_;

  //! Temporal offset of first spike (ms).
  double ignore_and_spike_offset_;

  //! Interval between two consecutive spikes (ms).
  double ignore_and_spike_interval_;

  //! Interval steps between two consecutive spikes.
  long ignore_and_spike_interval_steps_;

  //! Remaining steps until the next spike.
  long ignore_and_spike_steps_until_spike_;
};

}  // namespace nest

#endif  // IGNORE_AND_SPIKE_MECHANISM_H
