/*
 *  recording_backend_screen.cpp
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

// C++ includes:
#include <iostream>

// Includes from nestkernel:
#include "recording_device.h"

#include "recording_backend_screen.h"

void
nest::RecordingBackendScreen::enroll( const RecordingDevice& device,
				      const std::vector< Name >&,
				      const std::vector< Name >& )
{
  const index gid = device.get_gid();
  const thread t = device.get_thread();

  enrolled_devices_[ t ].insert( gid );
}

void
nest::RecordingBackendScreen::initialize()
{
  enrollment_map tmp( kernel().vp_manager.get_num_threads() );
  enrolled_devices_.swap( tmp );
}

void
nest::RecordingBackendScreen::finalize()
{
}

void
nest::RecordingBackendScreen::synchronize()
{
}

void
nest::RecordingBackendScreen::write( const RecordingDevice& device,
				     const Event& event,
				     const std::vector< double >& double_values,
				     const std::vector< long >& long_values )
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  if ( enrolled_devices_[ t ].find( gid ) == enrolled_devices_[ t ].end() )
  {
    return;
  }

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

#pragma omp critical
  {
    prepare_cout_();

    std::cout << sender << "\t";
    if ( device.get_time_in_steps() )
    {
      std::cout	<< stamp.get_steps() << "\t" << offset;
    }
    else
    {
      std::cout	<< stamp.get_ms() - offset;
    }
    for ( auto& val : double_values )
    {
      std::cout << "\t" << val;
    }
    for ( auto& val : long_values )
    {
      std::cout << "\t" << val;
    }
    std::cout << std::endl;

    restore_cout_();
  }
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendScreen::Parameters_::Parameters_()
  : precision_( 3 )
{
}

void
nest::RecordingBackendScreen::Parameters_::get( const RecordingBackendScreen&,
  DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
}

void
nest::RecordingBackendScreen::Parameters_::set( const RecordingBackendScreen&,
  const DictionaryDatum& d )
{
  if ( updateValue< long >( d, names::precision, precision_ ) )
  {
    std::cout << std::fixed;
    std::cout << std::setprecision( precision_ );
  }
}

void
nest::RecordingBackendScreen::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
