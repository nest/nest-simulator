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
  unsigned syn_id : 8;
  unsigned delay : 24;

  SynIdDelay( double d )
    : syn_id( invalid_synindex )
  {
    set_delay_ms( d );
  }

  SynIdDelay( const SynIdDelay& s )
    : syn_id( s.syn_id )
    , delay( s.delay )
  {
  }

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
};
}

#endif
