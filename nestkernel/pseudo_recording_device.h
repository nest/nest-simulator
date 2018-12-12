/*
 *  pseudo_recording_device.h
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

#ifndef PSEUDO_RECORDING_DEVICE_H
#define PSEUDO_RECORDING_DEVICE_H

// C++ includes:
#include <fstream>
#include <vector>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from nestkernel:
#include "device.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

/** @BeginDocumentation
  Name: PseudoRecordingDevice - Common properties of all pseudo-recording
                                devices.
  Description:

  Pseudo recording devices are used to measure properties of or signals emitted
  by network nodes. In contrast to fully flegded recording devices, they only
  register data in memory, but do not write data to file or screen.

  Parameters:
  The following parameters are shared with all devices:
  /start  - Actication time, relative to origin.
  /stop   - Inactivation time, relative to origin.
  /origin - Reference time for start and stop.

  SeeAlso: Device, StimulatingDevice, RecordingDevice
*/


/**
 * Base class for all pseudo recording devices.
 *
 * Pseudo-recording devices collect data. The data is only collected
 * in memory an only available through GetStatus.
 *
 * If the device is configured to record from start to stop, this
 * is interpreted as (start, stop], i.e., the earliest recorded
 * event will have time stamp start+1, as it was generated during
 * the update step (start, start+1].
 *
 * @note The sole purpose of this class is to provide an implementation
 *       of is_active().
 *
 * @ingroup Devices
 *
 * @author HEP 2002-07-22, 2008-03-21, 2008-07-01
 */
class PseudoRecordingDevice : public Device
{

public:
  PseudoRecordingDevice();
  PseudoRecordingDevice( const PseudoRecordingDevice& );
  virtual ~PseudoRecordingDevice()
  {
  }

  /** Indicate if recording device is active.
   *  The argument is the time stamp of the event, and the
   *  device is active if start_ < T <= stop_.
   */
  bool is_active( Time const& T ) const;
};

inline PseudoRecordingDevice::PseudoRecordingDevice()
  : Device()
{
}

inline PseudoRecordingDevice::PseudoRecordingDevice(
  const PseudoRecordingDevice& prd )
  : Device( prd )
{
}

inline bool
PseudoRecordingDevice::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();

  return get_t_min_() < stamp and stamp <= get_t_max_();
}

} // namespace

#endif // PSEUDO_RECORDING_DEVICE_H
