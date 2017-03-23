/*
 *  recording_backend_ascii.cpp
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

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "recording_device.h"
#include "vp_manager_impl.h"

#include "recording_backend_ascii.h"

void
nest::RecordingBackendASCII::enroll( RecordingDevice& device )
{
  std::vector< Name > value_names;
  enroll( device, value_names );
}

void
nest::RecordingBackendASCII::enroll( RecordingDevice& device, const std::vector< Name >& /* value_names */ )
{
  const int t = device.get_thread();
  const int gid = device.get_gid();

  // Ensure that a device is only enrolled once.
  assert(files_[ t ].find( gid ) == files_[ t ].end() );

  std::string filename = build_filename_( device );

  std::ifstream test( filename.c_str() );
  if ( ( test.good() && kernel().io_manager.overwrite_files() ) || not test.good() )
  {
    test.close();
    std::ofstream* file = new std::ofstream( filename.c_str() );

    if ( not file->good() )
    {
      std::string msg = String::compose(
        "I/O error while opening file '%1'. "
        "This may be caused by too many open files in networks "
        "with many recording devices and threads.",
        filename );
      LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
      throw IOError();
    }

    (*file) << std::fixed;
    (*file) << std::setprecision( P_.precision_ );

    files_[ t ].insert( std::make_pair( gid, std::make_pair( filename, file ) ) );
  }
  else
  {
    std::string msg = String::compose(
        "The device file '%1' exists already and will not be overwritten. "
        "Please change data_path, data_prefix or label, or set /overwrite_files "
        "to true in the root node.", filename );
    LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
    throw IOError();
  }
}

void
nest::RecordingBackendASCII::initialize()
{
  file_map tmp( kernel().vp_manager.get_num_threads() );
  files_.swap( tmp );
}

void
nest::RecordingBackendASCII::finalize()
{
  for ( size_t t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    file_map::value_type& inner = files_[t];
    for ( file_map::value_type::iterator f = inner.begin(); f != inner.end(); ++f )
    {
      f->second.second->close();
      delete f->second.second;
    }
  }
}

void
nest::RecordingBackendASCII::synchronize()
{
}

void
nest::RecordingBackendASCII::write( const RecordingDevice& device, const Event& event )
{
  int t = device.get_thread();
  int gid = device.get_gid();

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ t ][ gid ].second );
  file << sender << "\t" << stamp.get_ms() - offset << "\n";
}

void
nest::RecordingBackendASCII::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  int t = device.get_thread();
  int gid = device.get_gid();

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ t ][ gid ].second );
  file << sender << "\t" << stamp.get_ms() - offset;

  for ( std::vector< double >::const_iterator val = values.begin(); val != values.end(); ++val )
  {
    file << "\t" << *val;
  }

  file << "\n";
}

const std::string
nest::RecordingBackendASCII::build_filename_( const RecordingDevice& device ) const
{
  // number of digits in number of virtual processes
  const int vpdigits = static_cast< int >( std::floor( std::log10( static_cast< float >(
                                             kernel().vp_manager.get_num_virtual_processes() ) ) )
    + 1 );
  const int giddigits = static_cast< int >(
    std::floor( std::log10( static_cast< float >( kernel().node_manager.size() ) ) ) + 1 );

  std::ostringstream basename;
  const std::string& path = kernel().io_manager.get_data_path();
  if ( not path.empty() )
  {
    basename << path << '/';
  }
  basename << kernel().io_manager.get_data_prefix();

  const std::string& label = device.get_label();
  if ( not label.empty() )
  {
    basename << label;
  }
  else
  {
    basename << device.get_name();
  }

  int vp = device.get_vp();
  int gid = device.get_gid();

  basename << "-" << std::setfill( '0' ) << std::setw( giddigits ) << gid << "-"
           << std::setfill( '0' ) << std::setw( vpdigits ) << vp;

  return basename.str() + '.' + P_.file_ext_;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendASCII::Parameters_::Parameters_()
  : precision_( 3 )
  , file_ext_( "dat" )
{
}

void
nest::RecordingBackendASCII::Parameters_::get( const RecordingBackendASCII&, DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::file_extension ] = file_ext_;
}

void
nest::RecordingBackendASCII::Parameters_::set( const RecordingBackendASCII&, const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
  updateValue< std::string >( d, names::file_extension, file_ext_ );
}

void
nest::RecordingBackendASCII::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
