/*
 *  stimulation_backend.h
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

#ifndef STIMULATION_BACKEND_H
#define STIMULATION_BACKEND_H

// C++ includes:
#include <vector>

// Includes from sli:
#include "dictdatum.h"
#include "name.h"
#include "dictutils.h"
#include "stimulation_device.h"

namespace nest
{

/**
 * Abstract bass class for all NESTio stimulation backends
 *
 * This class provides the interface for NESTio stimulation backends
 * with which StimulationDevices can be enrolled for receiving
 * stimulation data that they can use to updating their parameters at
 * the beginning of each run.
 *
 * Built-in stimulation backends are registered in the constructor of
 * IOManager by inserting an instance of each of them into a std::map
 * under the name of the backend. The default backend, the one using
 * memory, are not registered in this map.
 *
 * A user level call to Simulate internally executes the sequence
 * Prepare → Run → Cleanup.  During Prepare, the prepare() function of
 * each backend is called by the IOManager. This gives the backend an
 * opportunity to prepare itself for being ready to receive the data.
 *
 * The user level function Run drives the simulation main loop by
 * updating all the stimulation device. At its beginning it calls
 * pre_run_hook() on each stimulation backend via the IOManager.
 * This function is used to receive or read data and update the
 * stimulation devices. At the end of each run, it calls post_run_hook()
 * on each stimulation backend via IOManager.
 *
 * During the simulation, stimulation backends do nothing. This solution
 * was chosen to avoid complex synchronization, but can be changed in the future
 * if the need shall arise.
 *
 * @author Sandra Diaz
 *
 * @ingroup NESTio
*/

class StimulationBackend
{
public:
  StimulationBackend() = default;

  virtual ~StimulationBackend() noexcept = default;

  /**
  * Enroll a `StimulationDevice` with the `StimulationBackend`.
  *
  * When this function is called by a `StimulationDevice` @p device,
  * the `StimulationBackend` can set up per-device data structures and
  * properties. Individual device instances can be identified using
  * the `thread` and `node_id` of the @p device.
  *
  * This function is called from the set_initialized_() function of
  * the @p device and their set_status() function.
  *
  * A backend needs to be able to cope with multiple calls to this
  * function, as multiple calls to set_status() may occur on the @p
  * device. For already enrolled devices this usually means that only
  * the parameters in @p params have to be set, but no further
  * actions are needed.
  *
  * Each stimulation backend must ensure that enrollment (including all
  * settings made by the user) is persistent over multiple calls to
  * Prepare, while the enrollment of all devices should end with a
  * call to finalize().
  *
  * A common implementation of this function will create an entry in
  * a thread-local map, associating the device's node ID with the
  * device-specific backend properties and an input facility of some
  * kind.
  *
  * @param device the StimulationDevice to be enrolled
  * @param params device-specific backend parameters
  *
  * @see disenroll()
  *
  * @ingroup NESTio
  */
  virtual void enroll( StimulationDevice&, const DictionaryDatum& ){};

  /**
   * Disenroll a `StimulationDevice` from the `StimulationBackend`.
   *
   * This function is considered to be the opposite of enroll() in the
   * sense that it cancels the enrollment of a StimulatinDevice from a
   * StimulationBackend by deleting all device specific data. When
   * setting a new stimulation backend for a stimulation device, this
   * function is called for each backend the device is not enrolled
   * with.
   *
   * @param device the StimulationDevice to be disenrolled
   *
   * @see enroll()
   *
   * @ingroup NESTio
   */
  virtual void disenroll( StimulationDevice& ){};

  /**
   * Initialize global backend-specific data structures.
   *
   * This function is called on each backend right at the very beginning of
   * `SimulationManager::run()`. It used for getting the data in order to update
   * the stimulation devices. The update of the device are made only if
   * necessary and repeated at the beginning of every single call to run in a
   * prepare-run-run-...-run-run-cleanup sequence.
   *
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
   * allows the backend to indicate that the run is ending.
   *
   * @see pre_run_hook()
   *
   * @ingroup NESTio
   */
  virtual void post_run_hook() = 0;

  // In order to allow a derived backend do work at the end of each simulation step, this base
  // class could define the function post_step_hook() right about here.
  // However, this function would be called at the very end of each simulation step and require
  // a very tight synchronization between incoming data and the simulation control itself. As the
  // requirements for this are currently not formally defined due to the lack of a suitable use-case,
  // we decided to omit the function from the interface until such a use-case arises.

  virtual void initialize() = 0;
  virtual void finalize() = 0;

  /**
   * Prepare the backend at the beginning of the NEST Simulate function.
   *
   * This function is called by `KernelManager::prepare()` and allows the
   * backend to open files, establish network connections, etc.
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
  * backend to close open files, close network connections, etc.
  *
  * @see prepare()
  *
  * @ingroup NESTio
  */
  virtual void cleanup() = 0;

  void clear( const StimulationDevice& ){};
};

} // namespace

#endif // STIMULATION_BACKEND_H
