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
 *
 * @note See model_details/ignore_and_spike_mechanism.rst for details.
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
   * Updates spike schedule and returns whether a spike should be emitted.
   *
   * @param emit_dynamic_spike Whether the neuron would emit a spike dynamically.
   * @return True if a spike should be emitted, false otherwise.
   */
  inline bool
  spike_event_is_due( bool emit_dynamic_spike )
  {
    if ( ignore_and_spike_ )
    {
      --steps_until_spike_;
      const bool emit_forced_spike = steps_until_spike_ == 0;
      if ( emit_forced_spike )
      {
        steps_until_spike_ = interval_steps_;
      }
      return emit_forced_spike;
    }
    else
    {
      return emit_dynamic_spike;
    }
  }

  /**
   * Retrieves parameters and adds them to the status dictionary.
   *
   * @param d Dictionary to which parameters are added.
   */
  void get_status( Dictionary& d ) const;

  /**
   * Sets and validates parameters from the status dictionary.
   *
   * @param d Dictionary from which parameters are set.
   */
  void set_status( const Dictionary& d );

private:
  /**
   * Initialize variables dependent on offset.
   */
  void initialize_offset();

  /**
   * Initialize variables dependent on interval.
   */
  void initialize_interval();

  //! If True, the neuron is forced to spike in set intervals.
  bool ignore_and_spike_;

  //! Temporal offset of first spike (ms).
  double offset_;

  //! Interval between two consecutive spikes (ms).
  double interval_;

  //! Interval steps between two consecutive spikes.
  long interval_steps_;

  //! Remaining steps until the next spike.
  long steps_until_spike_;
};

}  // namespace nest

#endif  // IGNORE_AND_SPIKE_MECHANISM_H
