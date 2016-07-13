/*
 *  io_backend_screen.cpp
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
#include "io_backend_screen.h"

void
nest::IOBackendScreen::enroll( RecordingDevice& )
{
}

void
nest::IOBackendScreen::enroll( RecordingDevice&, const std::vector< Name >& )
{
}

void
nest::IOBackendScreen::initialize()
{
  std::cout << std::fixed;
  std::cout << std::setprecision( P_.precision_ );
}

void
nest::IOBackendScreen::finalize()
{
}

void
nest::IOBackendScreen::synchronize()
{
}

void
nest::IOBackendScreen::write( const RecordingDevice& , const Event& event )
{
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

#pragma omp critical
  std::cout << sender << "\t" << stamp.get_ms() - offset << std::endl;
}

void
nest::IOBackendScreen::write( const RecordingDevice&,
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

nest::IOBackendScreen::Parameters_::Parameters_()
  : precision_( 3 )
{
}

void
nest::IOBackendScreen::Parameters_::get( const IOBackendScreen& , DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
}

void
nest::IOBackendScreen::Parameters_::set( const IOBackendScreen& , const DictionaryDatum& d )
{
  if ( updateValue< long >( d, names::precision, precision_ ) )
  {
    std::cout << std::fixed;
    std::cout << std::setprecision( precision_ );
  }
}

void
nest::IOBackendScreen::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
