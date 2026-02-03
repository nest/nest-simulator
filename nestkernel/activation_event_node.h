/*
 *  activation_event_node.h
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

#ifndef ACTIVATION_EVENT_NODE_H
#define ACTIVATION_EVENT_NODE_H

// nestkernel
#include "nest_time.h"

// libnestutil
#include "dict_util.h"

namespace nest
{
/**
 * @brief Base class providing activation event tracking functionality.
 *
 * This class provides the basic infrastructure for tracking activation events,
 * including the interval between activation events and the time of the last event.
 * It can be used by neurons that need to send periodic activation events.
 */
class ActivationEventNode
{
public:
  /**
   * Constructs a new ActivationEventNode object.
   */
  ActivationEventNode();

  /**
   * Constructs a new ActivationEventNode object by copying another.
   *
   * @param n The other object to copy.
   */
  ActivationEventNode( const ActivationEventNode& n );

  /**
   * Virtual destructor to enable proper cleanup in derived classes.
   */
  virtual ~ActivationEventNode() = default;

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
   * Retrieve activation event parameters and add to status dictionary.
   */
  void get_status( Dictionary& d ) const;

  /**
   * Set activation event parameters from status dictionary.
   * Validates that activation_interval > 0.
   */
  void set_status( const Dictionary& d );

protected:
  //! Interval between two activation events (ms).
  double activation_interval_;

  //! Time of last spike or activation event (steps).
  long last_event_time_;
};

} // namespace nest

#endif // ACTIVATION_EVENT_NODE_H
