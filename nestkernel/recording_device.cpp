/*
 *  recording_device.cpp
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

#include "recording_device.h"

#include "kernel_manager.h"

nest::RecordingDevice::Parameters_::Parameters_()
  : label_()
{
}

void
nest::RecordingDevice::Parameters_::get( const RecordingDevice& device, DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;

  kernel().io_manager.get_recording_backend()->get_device_status(device, d);
}

void
nest::RecordingDevice::Parameters_::set( const RecordingDevice&, const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::label, label_ );
}

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  Device::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
