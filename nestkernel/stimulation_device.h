/*
 *  stimulation_device.h
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

#ifndef STIMULATION_DEVICE_H
#define STIMULATION_DEVICE_H

// Includes from nestkernel:
#include "device.h"
#include "device_node.h"
#include "nest_types.h"

// Includes from sli:
#include "dictutils.h"

// Includes from libnestutil:
#include "compose.hpp"

// From standard library:
#include <string>

namespace nest
{

/* BeginUserDocs: NOINDEX

All stimulation devices share the parameters ``start`` and ``stop``,
which control the stimulation period. The property ``origin`` is a
global offset that shifts the stimulation period. All three values are
set as times in ms.

- For spike-emitting devices, only spikes with times `t` that fulfill
  `start` < `t` <= `stop` are emitted. Spikes that have timestamp of
  `t` = `start` are not emitted.

- For current-emitting devices, the current is activated and
  deactivated such that the current first affects the target dynamics
  during the update step (start, start+h], i.e., an effect can be
  recorded at the earliest at time start+h. The last interval during
  which the current affects the target's dynamics is (stop-h, stop].

The property ``stimulus_source`` defaults to an empty string. It can
be set to the name of a stimulation backend, in which case it will
take its parameters from the configured backend instead of from the
internally stored values. More details on available backends and their
properties can be found in the :ref:`guide to stimulating the network
<sec_stimulation_backends>`.

Parameters
++++++++++

label
    A string (default: `""`) specifying an arbitrary textual label for
    the device. Stimulation backends might use the label to generate
    device specific identifiers like filenames and such.

origin
    A positive floating point number (default : `0.0`) used as the
    reference time in ms for `start` and `stop`.

start
    A positive floating point number (default: `0.0`) specifying the
    activation time in ms, relative to `origin`.

stimulus_source
    A string (default: `""`) specifying the name of the stimulation
    backend from which to get the data for updating the stimulus
    parameters of the device. By default the device uses its
    internally stored parameters for updating the stimulus.

stop
    A floating point number (default: `infinity`) specifying the
    deactivation time in ms, relative to `origin`. The value of `stop`
    must be greater than or equal to `start`.

EndUserDocs */

/**
 * Base class for common properties of StimulationDevices.
 *
 * Stimulation devices are all devices injecting currents, spike trains
 * or other signals into a network. They provide only output and do not
 * receive any input.
 *
 * Stimulation devices come in (at least) two varieties: those providing
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
 * Since stimulation devices are connected to their targets with a delay of one
 * time step, this means that analog stimulation devices need to emit the event
 * during the update step for the interval (a-h, a]. Thus, the device needs
 * to be PRO-ACTIVE.
 *
 * Further, activity of stimulation devices is determined on the basis of
 * simulation time, not event time stamps. This means that the first simulation
 * time step during which the device must emit events is the step for which
 * the global clock has time a-h. If stimulation is to end by time b, this means
 * that the last event should be emitted during the time step for which the
 * global clock has time b-2h.
 *
 * @note Any stimulation devices transmitting analog signals must NOT HAVE
 * PROXIES.
 *
 * @note The distinction between analog and spike emitting devices is
 *       implemented by making StimulationDevice a template class with the type
 *       of the Event sent as template parameter. Member is_active() is not
 *       implemented in general and is available only for those cases for which
 *       it is explicitly specialized.
 *
 * @note StimulationDevice inherits protected from Device, so that
 *       implementations of is_active() can access t_min and t_max.
 *
 * @todo The timing of analog devices is correct only if they are transmitted
 *       using Network::send_local(), but we cannot enforce this currently.
 *
 * @ingroup Devices
 */
class StimulationDevice : public DeviceNode, public Device
{
public:
  StimulationDevice();
  StimulationDevice( StimulationDevice const& );
  ~StimulationDevice() override = default;

  /**
   * Determine whether device is active.
   * The argument is the value of the simulation time.
   * @see class comment for details.
   */
  bool is_active( const Time& ) const override;
  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& ) override;

  bool has_proxies() const override;
  Name get_element_type() const override;

  using Device::init_state;
  using Device::calibrate;
  using Device::init_buffers;
  using Node::calibrate;

  void calibrate() override;

  //! Throws IllegalConnection if synapse id differs from initial synapse id
  void enforce_single_syn_type( synindex );

  /**
   * Device type.
   */
  enum Type
  {
    CURRENT_GENERATOR,
    SPIKE_GENERATOR,
    DOUBLE_DATA_GENERATOR,
    DELAYED_RATE_CONNECTION_GENERATOR,
  };

  virtual Type
  get_type() const
  {
    throw KernelException( "WORNG TYPE" );
  };
  const std::string& get_label() const;
  virtual void set_data_from_stimulation_backend( std::vector< double >& ){};
  void update( Time const&, const long, const long ) override{};

protected:
  void set_initialized_() final;

  struct Parameters_
  {
    std::string label_;    //!< A user-defined label for symbolic device names.
    Name stimulus_source_; //!< Origin of the stimulation signal.

    Parameters_();
    Parameters_( const Parameters_& ) = default;
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  } P_;

private:
  /**
   * Synapse type of the first outgoing connection made by the Device.
   *
   * Used to check that devices connect using only a single synapse type,
   * see #481 and #737. Since this value must survive resets, it is
   * stored here, even though it is an implementation detail.
   */
  synindex first_syn_id_;

  DictionaryDatum backend_params_;
};

inline Name
StimulationDevice::get_element_type() const
{
  return names::stimulator;
}

inline bool
StimulationDevice::has_proxies() const
{
  return false;
}

} // namespace nest


#endif
