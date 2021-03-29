/*
 *  stimulating_backend.h
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

#ifndef STIMULATING_BACKEND_H
#define STIMULATING_BACKEND_H

// C++ includes:
#include <vector>

// Includes from sli:
#include "dictdatum.h"
#include "name.h"
#include "dictutils.h"
#include "stimulating_device.h"

namespace nest
{

class StimulatingBackend
{
public:
  StimulatingBackend() = default;

  virtual ~StimulatingBackend() noexcept = default;

  /**
  * Enroll a `StimulatingDevice` with the `StimulatingBackend`.
  *
  * When this function is called by a `StimulatingDevice` @p device,
  * the `StimulatingBackend` can set up per-device data structures and
  * properties. Individual device instances can be identified using
  * the `thread` and `node_id` of the @p device.
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
  * a thread-local map, associating the device's node ID with the
  * device-specific backend properties and an output facility of some
  * kind.
  *
  * @param device the StimulatingDevice to be enrolled
  * @param params device-specific backend parameters
  *
  * @see set_value_names(), disenroll(), write(),
  * @author Sandra Diaz
  *
  * @ingroup NESTio
  */
  virtual void enroll( StimulatingDevice&, const DictionaryDatum& ){};

  /**
   * Disenroll an `StimulatingDevice` from the `StimulatingBackend`.
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
  virtual void disenroll( StimulatingDevice& ){};

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

  void clear( const StimulatingDevice& ){};

  static const std::vector< Name > NO_DOUBLE_VALUE_NAMES;
  static const std::vector< Name > NO_LONG_VALUE_NAMES;
};

} // namespace

#endif // STIMULATING_BACKEND_H
