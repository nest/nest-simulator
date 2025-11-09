/*
 *  pseudo_recording_device.cpp
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

#include "pseudo_recording_device.h"

namespace nest
{

PseudoRecordingDevice::PseudoRecordingDevice()
  : Device()
{
}

PseudoRecordingDevice::PseudoRecordingDevice( const PseudoRecordingDevice& prd )
  : Device( prd )
{
}

bool
PseudoRecordingDevice::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();
  return get_t_min_() < stamp and stamp <= get_t_max_();
}

} // namespace nest
