/*
 *  stimulating_device.h
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

#ifndef STIMULATING_DEVICE_H
#define STIMULATING_DEVICE_H

// Includes from nestkernel:
#include "device.h"

// Includes from sli:
#include "dictutils.h"

class SpikeEvent;
class CurrentEvent;
class DoubleDataEvent;

/** @BeginDocumentation

   Name: StimulatingDevice - General properties of stimulating devices.

   Description:

   Stimulating devices inject signals into a network, either as analog signals
   such a currents or as spike trains. Most stimulating devices are implemented
   so that they are replicated on each virtual process. Many, but not all
   devices generating noise or stochastic spike trains provide different signals
   to each of their recipients; see the documentation of the individual device.

   Stimulating devices share the start, stop, and origin parameters global to
   devices. Start and stop have the following meaning for stimulating devices
   (origin is just a global offset):
   - For spike-emitting devices, only spikes with times t that fulfill
       start < t <= stop
     are emitted. Note that spikes that have time t==start are NOT emitted.
   - For current-emitting devices, the current is activated and deactivated such
     that the current first affects the target dynamics during the update step
     (start, start+h], i.e., an effect can be recorded at the earliest at time
     start+h. The last interval during which the current affects the target's
     dynamics is (stop-h, stop].

   Parameters:
   /start  - Actication time, relative to origin.
   /stop   - Inactivation time, relative to origin.
   /origin - Reference time for start and stop.

   SeeAlso: Device, RecordingDevice
*/

namespace nest
{

/**
 * Base class for common properties of Stimulating Devices.
 *
 * Stimulating devices are all devices injecting currents, spike trains
 * or other signals into a network. They provide only output and do not
 * receive any input.
 *
 * Stimulating devices come in (at least) two varieties: those providing
 * analog signals (CurrentEvent) and thos providing spike trains (SpikeEvent).
 * Device activation needs to be handled differently in both cases. The general
 * principle is that of the left-open, right-closed interval (start, stop].
 * For devices emitting spikes, spikes with times in that interval will be
 * emitted.
 *
 * For analog stimuli, e.g., currents, a stimulus is present in the
 * interval (t, t+h], where h is the simulation resolution, if the effect
 * of the stimulus can be observed at t+h. Thus, if a stimulus is to be
 * active from time a, its effect will first be observable at a+h. This
 * requires that the Event communicating the stimulus must be delivered
 * at time a, i.e., by the deliver_events() call prior to the update for
 * (a, a+h].
 *
 * Since stimulating devices are connected to their targets with a delay of one
 * time step, this means that analog stimulating devices need to emit the event
 * during the update step for the interval (a-h, a]. Thus, the device needs
 * to be PRO-ACTIVE.
 *
 * Further, activity of stimulating devices is determined on the basis of
 * simulation time, not event time stamps. This means that the first simulation
 * time step during which the device must emit events is the step for which
 * the global clock has time a-h. If stimulation is to end by time b, this means
 * that the last event should be emitted during the time step for which the
 * global clock has time b-2h.
 *
 * @note Any stimulating devices transmitting analog signals must NOT HAVE
 * PROXIES.
 *
 * @note The distinction between analog and spike emitting devices is
 *       implemented by making StimulatingDevice a template class with the type
 *       of the Event sent as template parameter. Member is_active() is not
 *       implemented in general and is available only for those cases for which
 *       it is explicitly specialized.
 *
 * @note StimulatingDevice inherits protected from Device, so that
 *       implementations of is_active() can access t_min and t_max.
 *
 * @todo The timing of analog devices is correct only if they are transmitted
 *       using Network::send_local(), but we cannot enforce this currently.
 *
 * @ingroup Devices
 */
template < typename EmittedEvent >
class StimulatingDevice : public Device
{
public:
  StimulatingDevice();
  StimulatingDevice( StimulatingDevice< EmittedEvent > const& );
  virtual ~StimulatingDevice()
  {
  }

  /**
   * Determine whether device is active.
   * The argument is the value of the simulation time.
   * @see class comment for details.
   */
  bool is_active( const Time& ) const;
  void get_status( DictionaryDatum& d ) const;

  //! Throws IllegalConnection if synapse id differs from initial synapse id
  void enforce_single_syn_type( synindex );

private:
  /**
   * Synapse type of the first outgoing connection made by the Device.
   *
   * Used to check that devices connect using only a single synapse type,
   * see #481 and #737. Since this value must survive resets, it is
   * stored here, even though it is an implementation detail.
   */
  synindex first_syn_id_;
};

template < typename EmittedEvent >
StimulatingDevice< EmittedEvent >::StimulatingDevice()
  : Device()
  , first_syn_id_( invalid_synindex )
{
}

template < typename EmittedEvent >
StimulatingDevice< EmittedEvent >::StimulatingDevice(
  StimulatingDevice< EmittedEvent > const& sd )
  : Device( sd )
  , first_syn_id_( invalid_synindex ) // a new instance can have no connections
{
}

// specializations must be declared inside namespace
template <>
inline bool
StimulatingDevice< nest::CurrentEvent >::is_active( const Time& T ) const
{
  /* We have t_min_ = origin_ + start_, t_max_ = origin_ + stop_ in steps.
     We need to check if
        t_min_ - 1 <= T.get_steps() <= t_max_ - 2
     This is equivalent to checking
        t_min_ <= T.get_steps() + 1 < t_max_
   */
  const long step = T.get_steps() + 1;
  return get_t_min_() <= step and step < get_t_max_();
}

template <>
inline bool
StimulatingDevice< nest::DoubleDataEvent >::is_active( const Time& T ) const
{
  // same as for the CurrentEvent
  const long step = T.get_steps() + 1;
  return get_t_min_() <= step and step < get_t_max_();
}

template <>
inline bool
StimulatingDevice< nest::SpikeEvent >::is_active( const Time& T ) const
{
  /* Input is the time stamp of the spike to be emitted. */
  const long stamp = T.get_steps();
  return get_t_min_() < stamp and stamp <= get_t_max_();
}

template < typename EmittedEvent >
inline void
StimulatingDevice< EmittedEvent >::get_status( DictionaryDatum& d ) const
{
  ( *d )[ names::element_type ] = LiteralDatum( names::stimulator );
  Device::get_status( d );
}

template < typename EmittedEvent >
inline void
nest::StimulatingDevice< EmittedEvent >::enforce_single_syn_type(
  synindex syn_id )
{
  if ( first_syn_id_ == invalid_synindex )
  {
    first_syn_id_ = syn_id;
  }
  if ( syn_id != first_syn_id_ )
  {
    throw IllegalConnection(
      "All outgoing connections from a device must use the same synapse "
      "type." );
  }
}
} // namespace nest

#endif
