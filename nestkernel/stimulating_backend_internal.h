/*
 *  stimulating_backend_internal.h
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

#ifndef STIMULATING_BACKEND_INTERNAL_H
#define STIMULATING_BACKEND_INTERNAL_H

#include "stimulating_backend.h"

/* BeginDocumentation

Internal stimulatingh backend
######################
Internal backend is the default backend. This backend does nothing.

@author Lionel Kusch

EndDocumentation */

namespace nest
{

/**
 * A simple input backend internal implementation
 */
class StimulatingBackendInternal : public StimulatingBackend
{
public:
  /**
   * InputBackend constructor
   * The actual initialization is happening in InputBackend::initialize()
   */
  StimulatingBackendInternal() = default;

  /**
   * InputBackend destructor
   * The actual finalization is happening in InputBackend::finalize()
   */
  ~StimulatingBackendInternal() noexcept override = default;


  void initialize() override;
  void finalize() override;

  //template<class T>
  //void enroll( StimulatingDevice<T>& device, const DictionaryDatum& params );

  //template<class T>
  //void disenroll( StimulatingDevice<T>& device );

  void cleanup() override;

  void prepare() override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void check_device_status( const DictionaryDatum& ) const override;
  template<class T>
  void set_value_names( const StimulatingDevice<T>& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names );

  void get_device_defaults( DictionaryDatum& ) const override;
  template<class T>
  void get_device_status( const StimulatingDevice<T>& device, DictionaryDatum& params_dictionary ) const;

private:
  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its input devices.
  */
  template<class T>
  using device_map= std::vector< std::map< int, const StimulatingDevice<T> > >;
  device_map<SpikeEvent> devices_;
};

} // namespace

#endif // STIMULATING_BACKEND_INTERNAL_H
