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

#include <iostream>
#include <iomanip>

#include "recording_device.h"
#include "recording_backend_screen.h"

void
nest::RecordingBackendScreen::enroll( RecordingDevice& )
{
}

void
nest::RecordingBackendScreen::enroll( RecordingDevice&, const std::vector< Name >& )
{
}

void
nest::RecordingBackendScreen::initialize()
{
  std::cout << std::fixed;
  std::cout << std::setprecision( P_.precision_ );
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
nest::RecordingBackendScreen::write( const RecordingDevice& , const Event& event )
{
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

#pragma omp critical
  std::cout << sender << "\t" << stamp.get_ms() - offset << std::endl;
}

void
nest::RecordingBackendScreen::write( const RecordingDevice&,
  const Event& event,
  const std::vector< double_t >& values )
{
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

#pragma omp critical
  {
    std::cout << sender << "\t" << stamp.get_ms() - offset;

    for ( std::vector< double_t >::const_iterator val = values.begin(); val != values.end(); ++val )
    {
      std::cout << "\t" << *val;
    }

    std::cout << std::endl;
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
nest::RecordingBackendScreen::Parameters_::get( const RecordingBackendScreen& , DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
}

void
nest::RecordingBackendScreen::Parameters_::set( const RecordingBackendScreen& , const DictionaryDatum& d )
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
