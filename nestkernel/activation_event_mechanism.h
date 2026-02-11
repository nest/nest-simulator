/*
 *  activation_event_mechanism.h
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

#ifndef ACTIVATION_EVENT_MECHANISM_H
#define ACTIVATION_EVENT_MECHANISM_H

// sli
#include "dictdatum.h"
#include "nest_time.h"

namespace nest
{
/**
 * @brief Class implementing an activation event mechanism for neuron models.
 *
 * This class implements an activation event mechanism for neuron models that
 * sends activation events after prolonged periods without spike emission.
 */
class ActivationEventMechanism
{
public:
  /**
   * Constructs a new ActivationEventMechanism object.
   */
  ActivationEventMechanism();

  /**
   * Constructs a new ActivationEventMechanism object by copying another.
   *
   * @param n The other object to copy.
   */
  ActivationEventMechanism( const ActivationEventMechanism& n );

  /**
   * Virtual destructor to enable proper cleanup in derived classes.
   */
  virtual ~ActivationEventMechanism() = default;

  /**
   * Sets the time the neuron last sent an event (spike or activation event).
   */
  void
  set_last_event_time( const long last_event_time )
  {
    last_event_time_ = last_event_time;
  }

  /**
   * Gets the last time the neuron sent an event (spike or activation event).
   */
  long
  get_last_event_time() const
  {
    return last_event_time_;
  }

  /**
   * Gets the interval between two activation events in steps.
   */
  long
  get_activation_interval_steps() const
  {
    return Time( Time::ms( activation_interval_ ) ).get_steps();
  }

  /**
   * Checks if an activation event is due at the current time.
   */
  bool
  is_activation_event_due( const long current_time ) const
  {
    return ( last_event_time_ > 0 and current_time - last_event_time_ ) >= get_activation_interval_steps();
  }

  /**
   * Retrieve activation event parameters and add to status dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set activation event parameters from status dictionary.
   * Validates that activation_interval > 0.
   */
  void set_status( const DictionaryDatum& d );

protected:
  //! Interval between two activation events (ms).
  double activation_interval_;

  //! Time of last spike or activation event (steps).
  long last_event_time_;
};

} // namespace nest

#endif // ACTIVATION_EVENT_MECHANISM_H
