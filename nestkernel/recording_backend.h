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
 * Abstract base class for all NESTio recording backends
 *
 * This class provides the interface for the NESTio recording backends
 * with which `RecordingDevice`s can be enrolled for recording and
 * which they can use to write their data.
 *
 * All recording backends are registered in the pre_run_hook() function 
 * of the IOManager by inserting an instance of each of them into a std::map
 * indexed by the name of the backend. A user level call to the NEST Simulate
 * function internally executes the sequence Prepare → Run → Cleanup. 
 * During Prepare, the pre_run_hook() function of each backend is called by the
 * IOManager. This gives the backend an opportunity to prepare data structures
 * for the upcoming simulation. The user level function Run runs the simulation
 * main loop and in every cycle updates all nodes and calls the function
 * IOManager::synchronize(), which in turn calls all backends’ synchronize()
 * function. During the simulation cycles, the recording devices might call 
 * the write() function of the IOManager. These calls are forwarded to all 
 * backends.At the end of Run the IOManager calls the post_run_hook() function
 * of every backend. Cleanup on the user level finally calls the cleanup()
 * function of all backends.
 *
 * Backend functions have to return as soon as possible if they have nothing to do.
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
   * When this function is called by a `RecordingDevice` @p device, the
   * `RecordingBackend` can set up or extend device-specific data
   * structures by using identifying properties of the @p device like
   * `thread` or `gid` that may be needed during the simulation phase
   * where data is written by the device.
   *
   * To make the names of recorded quantities known to the
   * `RecordingBackend`, the vectors @p double_value_names and @p
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
   * map itself would be created in the `pre_run_hook()` function.
   *
   * @param device the RecordingDevice to be enrolled
   * @param double_value_names the names for double values to be recorded
   * @param long_value_names the names for long values to be recorded
   *
   * @see write(), pre_run_hook()
   *
   * @ingroup NESTio
   */
  virtual void enroll( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) = 0;

  /**
   * Prepare the backend at begin of the NEST Simulate function.
   *
   * This function is called by `KernelManager::prepare()` and allows the
   * backend to open files or establish network connections or take similar
   * action.
   *
   * @see cleanup()
   *
   * @ingroup NESTio
   */
  virtual void prepare() = 0;

   /**
   * Clean up the backend at the end of a user level call to the NEST Simulate
   * function.
   *
   * This function is called by `SimulationManager::cleanup()` and allows the
   * backend to close open files or network connections or take similar action.
   *
   * @ingroup NESTio
   */
  virtual void cleanup() = 0;

  /**
   * Initialize global backend-specific data structures. 

   * This function is called on each backend on simulator startup as well as
   * upon changes in the number of threads. It is also called within Prepare.
   * As the number of threads can change in between calls to this function,
   * the backend can also re-structure its global data-structures accordingly.
   *
   * @see post_run_hook()
   * 
   * @ingroup NESTio
   */
  virtual void pre_run_hook() = 0;

   /**
   * Clean up the backend at the end of a Run.
   *
   * This is called right before `SimulationManager::run()` terminates. It
   * allows the backend to flush open files, write remaining data to the
   * screen, or perform similar operations that make sure that the user
   * has access to all data from the previous simulation run.
   *
   * @see pre_run_hook()
   *
   * @ingroup NESTio
   */
  virtual void post_run_hook() = 0;
 
   /**
   * Synchronize backends at the end of each simulation cycle.
   * 
   * This is called once per simulation cycle after all nodes have been
   * updated. Backends which record data collectively in a thread- or 
   * MPI-parallel manner can implement backend-specific synchronization
   * tasks here.
   *
   * @ingroup NESTio
   */
  virtual void synchronize() = 0;

  /**
   * Discard all recorded data.
   *
   * Called when the user sets the property
   * n_events on @p device to 0. This function only needs to be 
   * implemented by backends which store data of devices in memory.
   * 
   * @param device the RecordingDevice to be cleared
   *
   * @ingroup NESTio
   */
  virtual void clear( const RecordingDevice& device ) = 0;

  /**
   * Write the data from the event to the backend specific channel together
   * with the values given.
   *
   * This function needs to respect the time_in_steps property of the device
   * and should return as quickly as possible if the `RecordingDevice` @p device
   * is not enrolled with the backend.
   *
   * @param device the RecordingDevice, backend-specific channel to write to
   * @param event the event 
   * @param double_values vector of double valued to be written
   * @param long_values vector of long values to be written
   *
   * @ingroup NESTio
   */
  virtual void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& double_values,
    const std::vector< long >& long_values ) = 0;

  /**
   * Set the status of the recording backend using the key-value pairs
   * contained in the params dictionary.
   *
   * @param params_dictionary the status of the recording backend
   *
   * @see get_status()
   *
   * @ingroup NESTio
   */
  virtual void set_status( const DictionaryDatum& params_dictionary ) = 0;

  /**
   * Return the status of the recording backend by writing it to the given
   * params dictionary.
   *
   * @param params_dictionary the status of the recording backend
   *
   * @see set_status()
   *
   * @ingroup NESTio
   */  
  virtual void get_status( DictionaryDatum& params_dictionary ) const = 0;

  /**
   * Set the per-device status of the given recording device to the
   * key-value pairs contained in the params dictionary.
   *
   * @param device the recording device for which the status is to be set
   * @param params_dictionary the status of the recording device
   *
   * @see get_device_status()
   *
   * @ingroup NESTio
   */
  virtual void set_device_status( const RecordingDevice& device,
    const DictionaryDatum& params_dictionary ) = 0;

  /**
   * Return the per-device status of the given recording device by 
   * writing it to the given params dictionary.
   *
   * @param device the recording device for which the status is returned
   * @param params_dictionary the status of the recording device
   *
   * @see set_device_status()
   *
   * @ingroup NESTio
   */
  virtual void get_device_status( const RecordingDevice& device,
    DictionaryDatum& params_dictionary ) const = 0;

  static const std::vector< Name > NO_DOUBLE_VALUE_NAMES;
  static const std::vector< Name > NO_LONG_VALUE_NAMES;
  static const std::vector< double > NO_DOUBLE_VALUES;
  static const std::vector< long > NO_LONG_VALUES;
};

} // namespace

#endif // RECORDING_BACKEND_H
