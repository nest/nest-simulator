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

/**
 * Base class for all NESTio recording backends
 *
 * This class provides the interface for the NESTio recording backends
 * with which `RecordingDevice`s can be enrolled for recording and
 * which they can use to write their data.
*/

class RecordingBackend
{
public:
  RecordingBackend()
  {
  }

  virtual ~RecordingBackend() throw()
  {
  }

  /**
   * Enroll a `RecordingDevice` with the `RecordingBackend`.
   *
   * When this function is called by a RecordingDevice @p device, the
   * RecordingBackend can set up or extend device-specific data
   * structures by using identifying properties of the @p device like
   * `thread` or `gid` that may be needed during the simulation phase
   * where data is written by the device.
   *
   * To make the names of recorded quantities known to the
   * RecordingBackend, the vectors @p double_value_names and @p
   * long_value_names can be set appropriately. If no values of a
   * certain type (or none at all) will be recorded by @p device, the
   * constants @ref NO_DOUBLE_VALUE_NAMES and @ref NO_LONG_VALUE_NAMES
   * can be used. Please note that the lengths of the value names
   * vectors have to correspond to the length of the data vectors
   * written during calls to `write()`, although this is not enforced
   * by the API.
   *
   * A common implementation of this function will create an entry in
   * a map, associating the device and the name of its stored values
   * with some kind of output facility. In this case, the containing
   * map itself would be created in the initialize() function.
   *
   * @param device the RecordingDevice to be enrolled
   * @param double_value_names the names for double values to be recorded
   * @param long_value_names the names for long values to be recorded
   *
   * @see write(), initialize()
   * 
   * @ingroup NESTio
   */
  virtual void enroll( const RecordingDevice& device,
		       const std::vector< Name >& double_value_names,
		       const std::vector< Name >& long_value_names) = 0;

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
