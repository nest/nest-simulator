/*
 *  recording_backend_beep.cpp
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

//includes from sli:
#include "dictutils.h"

#include "recording_backend_beep.h"

nest::RecordingBackendBeep::RecordingBackendBeep()
{
}

nest::RecordingBackendBeep::~RecordingBackendBeep() throw()
{
  finalize();
}

void
nest::RecordingBackendBeep::enroll( const RecordingDevice& device )
{
  std::vector< Name > value_names;
  enroll( device, value_names );
}

void
nest::RecordingBackendBeep::enroll( const RecordingDevice& device,
  const std::vector< Name >& /* value_names */ )
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  file_map::value_type::iterator file_it = files_[ t ].find( gid );
  if ( file_it != files_[ t ].end() ) // already enrolled
  {
    files_[ t ].erase(file_it); 
  }

  std::string filename = build_filename_( device );
  
  std::ifstream test( filename.c_str() );
  if ( test.good() && not kernel().io_manager.overwrite_files() )
  {
    std::string msg = String::compose(
      "The device file '%1' exists already and will not be overwritten. "
      "Please change data_path, data_prefix or label, or set /overwrite_files "
      "to true in the root node.", filename );
    LOG( M_ERROR, "RecordingDevice::calibrate()", msg );

    files_[ t ].insert( std::make_pair( gid,
      std::make_pair( filename, static_cast< std::ofstream* >( NULL ) ) ) );

    throw IOError();
  }
  test.close();
  
  std::ofstream* file = new std::ofstream( filename.c_str() );
  ( *file ) << std::fixed << std::setprecision( P_.precision_ );

  if ( not file->good() )
  {
    std::string msg = String::compose(
      "I/O error while opening file '%1'. "
      "This may be caused by too many open files in networks "
      "with many recording devices and threads.", filename );
    LOG( M_ERROR, "RecordingDevice::calibrate()", msg );

    files_[ t ].insert( std::make_pair( gid,
      std::make_pair( filename, static_cast< std::ofstream* >( NULL ) ) ) );

    throw IOError();
  }
  
  //enroll the device
  files_[ t ].insert( std::make_pair( gid, std::make_pair( filename, file ) ) );
}

void
nest::RecordingBackendBeep::initialize()
{
  file_map tmp( kernel().vp_manager.get_num_threads() );
  files_.swap( tmp );
}

void
nest::RecordingBackendBeep::post_run_cleanup()
{
  for ( size_t t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    file_map::value_type& inner = files_[ t ];
    for ( file_map::value_type::iterator f = inner.begin(); f != inner.end();
          ++f )
    {
      f->second.second->flush();
    }
  }
}

void
nest::RecordingBackendBeep::finalize()
{
  for ( size_t t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    file_map::value_type& inner = files_[ t ];
    file_map::value_type::iterator f;
    for ( f = inner.begin(); f != inner.end(); ++f )
    {
      if ( f->second.second != NULL )
      {
	f->second.second->close();
	delete f->second.second;
	f->second.second = NULL;
      }
    }
  }
}

void
nest::RecordingBackendBeep::synchronize()
{
}

void
nest::RecordingBackendBeep::write( const RecordingDevice& device,
  const Event& event )
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  if ( files_[ t ].find( gid ) == files_[ t ].end() )
  {
    return;
  }

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ t ][ gid ].second );
  file << sender << "\t";
  if ( device.get_time_in_steps() )
  {
    file << stamp.get_steps() << "\t" << offset;
  }
  else
  {
    file << stamp.get_ms() - offset;
  }
  file << "\n";
}

void
nest::RecordingBackendBeep::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double >& values )
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  if ( files_[ t ].find( gid ) == files_[ t ].end() )
  {
    return;
  }

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ t ][ gid ].second );
  file << sender << "\t";
  if ( device.get_time_in_steps() )
  {
    file << stamp.get_steps() << "\t" << offset;
  }
  else
  {
    file << stamp.get_ms() - offset;
  }
  std::vector< double >::const_iterator val;
  for ( val = values.begin(); val != values.end(); ++val )
  {
    file << "\t" << *val;
  }
  file << "\n";
}

const std::string
nest::RecordingBackendBeep::build_filename_(
  const RecordingDevice& device ) const
{
  // number of digits in number of virtual processes
  const int vpdigits = static_cast< int >(
    std::floor( std::log10( static_cast< float >(
      kernel().vp_manager.get_num_virtual_processes() ) ) ) + 1 );
  const int giddigits = static_cast< int >(
    std::floor( std::log10(
      static_cast< float >( kernel().node_manager.size() ) ) ) + 1 );

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

  basename << "-" << std::setfill( '0' ) << std::setw( giddigits ) << gid
	   << "-" << std::setfill( '0' ) << std::setw( vpdigits ) << vp;

  return basename.str() + '.' + P_.file_ext_;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::RecordingBackendBeep::Parameters_::Parameters_()
  : precision_( 3 )
  , file_ext_( "dat" )
{
}

void
nest::RecordingBackendBeep::Parameters_::get( const RecordingBackendBeep&,
  DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::file_extension ] = file_ext_;
}

void
nest::RecordingBackendBeep::Parameters_::set( const RecordingBackendBeep&,
  const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
  updateValue< std::string >( d, names::file_extension, file_ext_ );
}

void
nest::RecordingBackendBeep::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

void
nest::RecordingBackendBeep::get_device_status( const RecordingDevice& device,
  DictionaryDatum& d ) const
{
  const thread t = device.get_thread();
  const index gid = device.get_gid();

  file_map::value_type::const_iterator file = files_[ t ].find( gid );
  if ( file != files_[ t ].end() )
  {
    initialize_property_array( d, names::filenames );
    append_property( d, names::filenames, file->second.first );
  }
}
