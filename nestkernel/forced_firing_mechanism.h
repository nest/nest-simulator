/*
 *  forced_firing_mechanism.h
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

#ifndef FORCED_FIRING_MECHANISM_H
#define FORCED_FIRING_MECHANISM_H

// nestkernel
#include "nest_time.h"

// libnestutil
#include "dict_util.h"

namespace nest
{
/**
 * @brief Class implementing a forced firing mechanism for neuron models.
 *
 * This class implements a forced firing mechanism.
 */
class ForcedFiringMechanism
{
public:
  /**
   * Default constructor.
   */
  ForcedFiringMechanism();

  /**
   * Copy constructor.
   *
   * @param n The other object to copy.
   */
  ForcedFiringMechanism( const ForcedFiringMechanism& n );

  /**
   * Virtual destructor.
   */
  virtual ~ForcedFiringMechanism() = default;

  /**
   * Re-calculates dependent parameters.
   */
  void pre_run_hook();

  /**
   * Checks if a forced firing is due at the current time.
   */
  bool emit_spike( bool );

  /**
   * Retrieves parameters and adds them to the status dictionary.
   */
  void get_status( Dictionary& d ) const;

  /**
   * Sets and validates parameters from the status dictionary.
   */
  void set_status( const Dictionary& d );

protected:
  //! If True, the neuron runs in ignore-and-fire mode.
  bool forced_firing_;

  //! Temporal offset of forced firing in ignore-and-fire mode (ms).
  double forced_firing_offset_;

  //! Rate of forced firing in ignore-and-fire mode (spikes/s).
  double forced_firing_rate_;

  //! Remaining steps until the next forced spike.
  long steps_until_forced_spike_;

  //! Steps between two consecutive forced spikes.
  long forced_firing_interval_steps_;
};

}  // namespace nest

#endif  // FORCED_FIRING_MECHANISM_H
