/*
 *  syn_id_delay.h
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

#ifndef SYN_ID_DELAY_H
#define SYN_ID_DELAY_H

// Includes from nestkernel:
#include "nest_time.h"
#include "nest_types.h"

namespace nest
{

struct SynIdDelay
{
  unsigned int delay : NUM_BITS_DELAY;
  unsigned int syn_id : NUM_BITS_SYN_ID;
  bool more_targets : 1;
  bool disabled : 1;

  explicit SynIdDelay( double d )
    : syn_id( invalid_synindex )
    , more_targets( false )
    , disabled( false )
  {
    set_delay_ms( d );
  }

  SynIdDelay( const SynIdDelay& s ) = default;

  /**
   * Return the delay of the connection in ms
   */
  double
  get_delay_ms() const
  {
    return Time::delay_steps_to_ms( delay );
  }

  /**
   * Set the delay of the connection specified in ms
   */
  void
  set_delay_ms( const double d )
  {
    delay = Time::delay_ms_to_steps( d );
  }

  void
  set_source_has_more_targets( const bool more_targets )
  {
    this->more_targets = more_targets;
  }

  bool
  source_has_more_targets() const
  {
    return more_targets;
  }

  /**
   * Disables the synapse.
   *
   * @see is_disabled
   */
  void
  disable()
  {
    disabled = true;
  }

  /**
   * Returns a flag denoting if the synapse is disabled.
   *
   * @see disable
   */
  bool
  is_disabled() const
  {
    return disabled;
  }
};

//! check legal size
using success_syn_id_delay_data_size = StaticAssert< sizeof( SynIdDelay ) == 4 >::success;
}

#endif
