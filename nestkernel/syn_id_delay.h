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

namespace nest
{

struct SynIdDelay
{
  unsigned int delay : 23;
  unsigned int syn_id : 8;
  bool subsequent_targets : 1;

  SynIdDelay( double_t d )
    : syn_id( invalid_synindex )
    , subsequent_targets( 0 )
  {
    set_delay_ms( d );
  }

  SynIdDelay( const SynIdDelay& s )
    : delay( s.delay )
    , syn_id( s.syn_id )
    , subsequent_targets( s.subsequent_targets )
  {
  }

  /**
   * Return the delay of the connection in ms
   */
  double_t
  get_delay_ms() const
  {
    return Time::delay_steps_to_ms( delay );
  }

  /**
   * Set the delay of the connection specified in ms
   */
  void
  set_delay_ms( const double_t d )
  {
    delay = Time::delay_ms_to_steps( d );
  }

  void
  set_has_source_subsequent_targets( const bool subsequent_targets )
  {
    this->subsequent_targets = subsequent_targets;
  }

  bool
  has_source_subsequent_targets() const
  {
    return this->subsequent_targets;
  }

};
}

#endif
