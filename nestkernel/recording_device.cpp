/*
 *  recording_device.cpp
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

#include "recording_device.h"

nest::RecordingDevice::Parameters_::Parameters_()
  : filename_()
  , label_()
{
}

void
nest::RecordingDevice::Parameters_::get( const RecordingDevice& sl, DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  
  if ( !filename_.empty() )
  {
    initialize_property_array( d, names::filenames );
    append_property( d, names::filenames, filename_ );
  }
}

void
nest::RecordingDevice::Parameters_::set( const RecordingDevice& sl, const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::label, label_ );
}

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;    // temporary copy in case of errors
  ptmp.set( *this, d ); // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;

  //if ( P_.to_file_ && B_.fs_.is_open() ) // TODO: check if this is neccessary
  P_.filename_.clear();
}
