/*
 *  recording_backend_sionlib.h
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

namespace nest
{

class RecordingBackendArbor : public RecordingBackend
{
public:
  RecordingBackendArbor();
  ~RecordingBackendArbor() throw();

  void enroll( const RecordingDevice& device );
  // should never be called (below)
  void enroll( const RecordingDevice& device,
    const std::vector< Name >& value_names );

  void finalize();
  void synchronize();

  void prepare();
  void cleanup();

  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  void initialize();
  void calibrate();

private:
  bool prepared_;
  bool cleanedup_;

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

inline void
RecordingBackendArbor::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );
}

} // namespace

#endif // RECORDING_BACKEND_ARBOR_H
