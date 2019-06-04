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

  virtual void enroll( const RecordingDevice&,
		       const std::vector< Name >&,
		       const std::vector< Name >&) = 0;

  virtual void
  pre_run_hook() = 0;

  virtual void
  prepare()
  {
  }

  virtual void
  post_run_hook()
  {
  }

  virtual void cleanup() = 0;
  virtual void synchronize() = 0;

  virtual void
  clear( const RecordingDevice& )
  {
  }

  virtual void write( const RecordingDevice&,
		      const Event&,
		      const std::vector< double >&,
		      const std::vector< long >& ) = 0;

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

  static const std::vector< Name > NO_DOUBLE_VALUE_NAMES;
  static const std::vector< Name > NO_LONG_VALUE_NAMES;
  static const std::vector< double > NO_DOUBLE_VALUES;
  static const std::vector< long > NO_LONG_VALUES;
  
};

} // namespace

#endif // RECORDING_BACKEND_H
