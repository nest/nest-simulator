/*
 *  input_backend_internal.h
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

#ifndef INPUT_BACKEND_INTERNAL_H
#define INPUT_BACKEND_INTERNAL_H

#include "input_backend.h"


namespace nest
{

/**
 * A simple input backend internal implementation
 */
class InputBackendInternal : public InputBackend
{
public:
  /**
   * InputBackend constructor
   * The actual initialization is happening in RecordingBackend::initialize()
   */
  InputBackendInternal()=default;

  /**
   * InputBackend destructor
   * The actual finalization is happening in RecordingBackend::finalize()
   */
  ~InputBackendInternal() noexcept = default;


  void initialize() override;
  void finalize() override;

  void enroll( InputDevice& device, const DictionaryDatum& params ) override;

  void disenroll( InputDevice& device ) override;

  void cleanup() override;

  void prepare() override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void check_device_status( const DictionaryDatum& ) const override;
  void set_value_names( const InputDevice& device,
  const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names ) override;

  void get_device_defaults( DictionaryDatum& ) const override;
  void get_device_status( const InputDevice& device, DictionaryDatum& params_dictionary ) const override;

private:

  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its recordings.
  */
  typedef std::vector< std::map< int, const InputDevice* > > device_map;
  device_map devices_;

};

} // namespace

#endif // INPUT_BACKEND_INTERNAL_H
