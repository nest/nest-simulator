/*
 *  flush_event_mechanism.h
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

#ifndef FLUSH_EVENT_MECHANISM_H
#define FLUSH_EVENT_MECHANISM_H

// nestkernel
#include "nest_time.h"

// libnestutil
#include "dict_util.h"

namespace nest
{
/**
 * @brief Class implementing a flush event mechanism for neuron models.
 *
 * This class implements a flush event mechanism for neuron models that
 * sends flush events after prolonged periods without spike emission.
 */
class FlushEventMechanism
{
public:
  /**
   * Constructs a new FlushEventMechanism object.
   */
  FlushEventMechanism();

  /**
   * Constructs a new FlushEventMechanism object by copying another.
   *
   * @param n The other object to copy.
   */
  FlushEventMechanism( const FlushEventMechanism& n );

  /**
   * Virtual destructor to enable proper cleanup in derived classes.
   */
  virtual ~FlushEventMechanism() = default;

  /**
   * Sets the time the neuron last sent an event (spike or flush event).
   */
  void
  set_last_event_time( const long last_event_time )
  {
    last_event_time_ = last_event_time;
  }

  /**
   * Gets the last time the neuron sent an event (spike or flush event).
   */
  long
  get_last_event_time() const
  {
    return last_event_time_;
  }

  /**
   * Gets the interval between two events in steps.
   */
  long
  get_flush_event_send_interval_steps() const
  {
    return Time( Time::ms( flush_event_send_interval_ ) ).get_steps();
  }

  /**
   * Checks if an flush event is due at the current time.
   */
  bool
  flush_event_is_due( const long current_time ) const
  {
    return ( last_event_time_ > 0 and current_time - last_event_time_ ) >= get_flush_event_send_interval_steps();
  }

  /**
   * Retrieve flush event parameters and add to status dictionary.
   */
  void get_status( Dictionary& d ) const;

  /**
   * Set flush event parameters from status dictionary.
   * Validates that flush_event_send_interval > 0.
   */
  void set_status( const Dictionary& d );

protected:
  //! Interval since previous event after which a flush event is sent (ms).
  double flush_event_send_interval_;

  //! Time of last spike or flush event (steps).
  long last_event_time_;
};

} // namespace nest

#endif // FLUSH_EVENT_MECHANISM_H
