/*
 *  recording_backend_arbor.h
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

#ifndef RECORDING_BACKEND_ARBOR_H
#define RECORDING_BACKEND_ARBOR_H

// C includes:
#include <memory>

#include "recording_backend.h"
#include "mpiutil.h"

namespace nest
{

class RecordingBackendArbor : public RecordingBackend
{
public:
  RecordingBackendArbor();
  ~RecordingBackendArbor() throw();

  void initialize() override;
  void finalize() override;

  void enroll( const RecordingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) override;

  void cleanup() override;

  void prepare() override;

  void write( const RecordingDevice&, const Event&, const std::vector< double >&, const std::vector< long >& ) override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void check_device_status( const DictionaryDatum& ) const override;
  void get_device_defaults( DictionaryDatum& ) const override;
  void get_device_status( const RecordingDevice& device, DictionaryDatum& params_dictionary ) const override;

private:
  void exchange_( std::vector< arb::shadow::spike >& );

  bool enrolled_;
  bool prepared_;

  int steps_left_;
  unsigned arbor_steps_;
  unsigned num_arbor_cells_;

  std::unique_ptr< struct ArborInternal > arbor_;

  typedef std::vector< std::map< index, const RecordingDevice* > > device_map;
  device_map devices_;

  struct Parameters_
  {
    Parameters_();

    void get( const RecordingBackendArbor&, DictionaryDatum& ) const;
    void set( const RecordingBackendArbor&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

} // namespace

#endif // RECORDING_BACKEND_ARBOR_H
