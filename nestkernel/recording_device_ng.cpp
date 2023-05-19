/*
 *  recording_device_ng.cpp
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

// Includes from libnestutil:
#include "compose.hpp"
#include "kernel_manager.h"

#include "recording_device_ng.h"


nest::RecordingDeviceNG::RecordingDeviceNG()
  : NESTObjectInterface()
  , Device()
  , P_()
  , backend_params_( new Dictionary )
{
}


nest::RecordingDeviceNG::RecordingDeviceNG( const RecordingDeviceNG& rd )
  : NESTObjectInterface( rd )
  , Device( rd )
  , P_( rd.P_ )
  , backend_params_( new Dictionary( *rd.backend_params_ ) )
{
}
