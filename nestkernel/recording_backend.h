/*
 *  recording_backend.h
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

#ifndef RECORDING_BACKEND_H
#define RECORDING_BACKEND_H

// C++ includes:
#include <vector>

// Includes from sli:
#include "dictdatum.h"
#include "name.h"


namespace nest
{

class RecordingDevice;
class Event;

class RecordingBackend
{
public:
  RecordingBackend()
  {
  }

  virtual ~RecordingBackend() throw()
  {
  }

  virtual void enroll( const RecordingDevice& device ) = 0;
  virtual void enroll( const RecordingDevice& device,
    const std::vector< Name >& value_names ) = 0;

  virtual void
  initialize() = 0;

  virtual void
  post_run_cleanup()
  {
  }

  virtual void finalize() = 0;
  virtual void synchronize() = 0;

  virtual void
  clear( const RecordingDevice& )
  {
  }

  virtual void write( const RecordingDevice& device, const Event& event ) = 0;
  virtual void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& ) = 0;

  virtual void
  set_status( const DictionaryDatum& )
  {
  }

  virtual void
  get_status( DictionaryDatum& ) const
  {
  }

  virtual void
  set_device_status( const RecordingDevice& device, const DictionaryDatum& d )
  {
  }

  virtual void
  get_device_status( const RecordingDevice& device, DictionaryDatum& d ) const
  {
  }
};

} // namespace

#endif // RECORDING_BACKEND_H
