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
 * Built-in recording backends are registered in the constructor of
 * IOManager by inserting an instance of each of them into a std::map
 * under the name of the backend.
 *
 * A user level call to Simulate internally executes the sequence
 * Prepare → Run → Cleanup.  During Prepare, the prepare() function of
 * each backend is called by the IOManager. This gives the backend an
 * opportunity to prepare data structures for the upcoming
 * simulation cycle.
 *
 * The user level function Run drives the simulation main loop by
 * uptating all nodes. At its beginning it calls pre_run_hook() on
 * each recording backend via the IOManager. At the end of each run,
 * it calls post_run_hook() respectively.
 *
 * During the simulation, recording devices call IOManager::write() in
 * order to record data. These calls are forwarded to the backend, the
 * device is enrolled with. Cleanup on the user level finally calls
 * the cleanup() function of all backends.
 *
 * @ingroup NESTio
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

  virtual void initialize() = 0;
  virtual void finalize() = 0;

  /**
   * Enroll a `RecordingDevice` with the `RecordingBackend`.
   *
   * When this function is called by a `RecordingDevice` @p device,
   * the `RecordingBackend` can set up per-device data structures and
   * properties. Individual device instances can be identified using
   * the `thread` and `gid` of the @p device.
   *
   * This function is called from the set_initialized_() function of
   * the @p device and their set_status() function. The companion
   * function @p set_value_names() is called from Node::pre_run_hook()
   * and makes the names of values to be recorded known.
   *
   * A backend needs to be able to cope with multiple calls to this
   * function, as multiple calls to set_status() may occur on the @p
   * device. For already enrolled devices this usually means that only
   * the parameters in @p params have to be set, but no further
   * actions are needed.
   *
   * Each recording backend must ensure that enrollment (including all
   * settings made by the user) is persistent over multiple calls to
   * Prepare, while the enrollment of all devices should end with a
   * call to finalize().
   *
   * A common implementation of this function will create an entry in
   * a thread-local map, associating the device's global id with the
   * device-specific backend properties and an output facility of some
   * kind.
   *
   * @param device the RecordingDevice to be enrolled
   * @param params device-specific backend parameters
   *
   * @see set_value_names(), disenroll(), write(),
   *
   * @ingroup NESTio
   */
  virtual void enroll( const RecordingDevice& device, const DictionaryDatum& params ) = 0;

  /**
   * Disenroll a `RecordingDevice` from the `RecordingBackend`.
   *
   * This function is considered to be the opposite of enroll() in the
   * sense that it cancels the enrollment of a RecordingDevice from a
   * RecordingBackend by deleting all device specific data. When
   * setting a new recording backend for a recording device, this
   * function is called for each backend the device is not enrolled
   * with.
   *
   * @param device the RecordingDevice to be disenrolled
   *
   * @see enroll()
   *
   * @ingroup NESTio
   */
  virtual void disenroll( const RecordingDevice& device ) = 0;

  /**
   * To make the names of recorded quantities known to the
   * `RecordingBackend`, the vectors @p double_value_names and @p
   * long_value_names can be set appropriately. If no values of a
   * certain type (or none at all) will be recorded by @p device, the
   * constants @ref NO_DOUBLE_VALUE_NAMES and @ref NO_LONG_VALUE_NAMES
   * can be used. Please note that the lengths of the value names
   * vectors *must* correspond to the length of the data vectors
   * written during calls to `write()`, although this is not enforced
   * by the API.
   *
   * @param device the device to set the value names for
   * @param double_value_names the names for double values to be recorded
   * @param long_value_names the names for long values to be recorded
   *
   * @see enroll(), disenroll(), write(),
   *
   * @ingroup NESTio
   */
  virtual void set_value_names( const RecordingDevice& device,
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
  * @see prepare()
  *
  * @ingroup NESTio
  */
  virtual void cleanup() = 0;

  /**
   * Initialize global backend-specific data structures.
   *
   * This function is called on each backend right at the very beginning of
   * `SimulationManager::run()`. It can be used for initializations which have
   * to be repeated at the beginning of every single call to run in a
   * prepare-run-run-...-run-run-cleanup sequence.
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
   * Do work required at the end of each simulation step.
   *
   * This is called at the very end of each simulation step. It can for example
   * be used to carry out writing to files in a synchronized way, all threads
   * on all MPI processes performing it at the same time.
   *
   * @see pre_run_hook()
   *
   * @ingroup NESTio
   */
  virtual void post_step_hook() = 0;

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
   * @param params the status of the recording backend
   *
   * @see get_status()
   *
   * @ingroup NESTio
   */
  virtual void set_status( const DictionaryDatum& params ) = 0;

  /**
   * Return the status of the recording backend by writing it to the given
   * params dictionary.
   *
   * @param params the status of the recording backend
   *
   * @see set_status()
   *
   * @ingroup NESTio
   */
  virtual void get_status( DictionaryDatum& params ) const = 0;

  /**
   * Check if the given per-device properties are valid and usable by
   * the backend.
   *
   * This function is used to validate properties when SetDefaults is
   * called on a recording device. If the properties are found to be
   * valid, they will be cached in the recording device and set for
   * individual instances by means of the call to enroll from the
   * device's set_initialized_() function. In case the properties are
   * invalid, this function is expected to throw BadProperty.
   *
   * @param params the parameter dictionary to validate
   *
   * @see get_device_defaults(), get_device_status()
   *
   * @ingroup NESTio
   */
  virtual void check_device_status( const DictionaryDatum& params ) const = 0;

  /**
   * Return the per-device defaults by writing it to the given params
   * dictionary.
   *
   * @param params the dictionary to add device-specific backend parameters to
   *
   * @see check_device_status(), get_device_status()
   *
   * @ingroup NESTio
   */
  virtual void get_device_defaults( DictionaryDatum& params ) const = 0;

  /**
   * Return the per-device status of the given recording device by
   * writing it to the given params dictionary.
   *
   * Please note that a corresponding setter function does not exist.
   * Device-specific backend parameters are given in the call to
   * enroll.
   *
   * @param device the recording device for which the status is returned
   * @param params the dictionary to add device-specific backend parameters to
   *
   * @see enroll(), check_device_status(), get_device_defaults()
   *
   * @ingroup NESTio
   */
  virtual void get_device_status( const RecordingDevice& device, DictionaryDatum& params ) const = 0;

  static const std::vector< Name > NO_DOUBLE_VALUE_NAMES;
  static const std::vector< Name > NO_LONG_VALUE_NAMES;
  static const std::vector< double > NO_DOUBLE_VALUES;
  static const std::vector< long > NO_LONG_VALUES;
};

} // namespace

#endif // RECORDING_BACKEND_H
