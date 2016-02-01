/*
 *  ascii_logger.cpp
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

#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>

#include "kernel_manager.h"
#include "recording_device.h"
#include "ascii_logger.h"
#include "vp_manager_impl.h"
#include "compose.hpp"

#include "dictutils.h"

void
nest::ASCIILogger::enroll( RecordingDevice& device )
{
  std::vector< Name > value_names;
  enroll( device, value_names );
}

void
nest::ASCIILogger::enroll( RecordingDevice& device, const std::vector< Name >&  )
{
  const int task = device.get_vp();
  const int gid = device.get_gid();

#pragma omp critical
  {
    // Check if the map already contains a submap for the task the device is instantiated on. Create
    // it if that is not the case.
    if ( files_.find( task ) == files_.end() )
    {
      files_.insert( std::make_pair( task, file_map::mapped_type() ) );
    }
  }

  // Insert the device with a newly created file stream into the thread-local map.
  // Devices can not be enrolled more than once.
  assert( files_[ task ].find( gid ) == files_[ task ].end() );
  files_[ task ].insert( std::make_pair( gid, std::make_pair( &device, new std::ofstream() ) ) );
}

void
nest::ASCIILogger::initialize()
{
  // we need to delay the throwing of exceptions to the end of the parallel section
  WrappedThreadException* we = NULL;

#pragma omp parallel
  {
    thread t = kernel().vp_manager.get_thread_id();
    thread task = kernel().vp_manager.thread_to_vp( t );

    try
    {
#pragma omp critical
      {
        // Insert an empty map to guarantee its existance and allow simpler handling at later
        // points.
        if ( files_.find( task ) == files_.end() )
        {
          files_.insert( std::make_pair( task, file_map::mapped_type() ) );
        }
      }
#pragma omp barrier

      // extract the inner map (containing the registered devices) for the specific VP
      typedef file_map::mapped_type inner_map;
      inner_map inner = files_.find( task )->second;

      // iterate over registed devices and their corresponding file streams
      for ( inner_map::iterator jj = inner.begin(); jj != inner.end(); ++jj )
      {
        //int gid = jj->first;
        std::ofstream& file = *( jj->second.second );
        RecordingDevice& device = *( jj->second.first );

        // initialize file according to parameters
        std::string filename;

        // do we need to (re-)open the file
        bool newfile = false;

        if ( !file.is_open() )
        {
          newfile = true; // no file from before
          filename = build_filename_( device );
          device.set_filename( filename );
        }
        else
        {
          std::string newname = build_filename_( device );
          if ( newname != device.get_filename() )
          {
#ifndef NESTIO
            std::string msg = String::compose(
              "Closing file '%1', opening file '%2'", device.get_filename(), newname );
            LOG( M_INFO, "RecordingDevice::calibrate()", msg );
#endif // NESTIO

            file.close(); // close old file
            device.set_filename( newname );
            newfile = true;
          }
        }

        if ( newfile )
        {
          assert( !file.is_open() );
          if ( kernel().io_manager.overwrite_files() )
          {
            file.open( filename.c_str() );
          }
          else
          {
            // try opening for reading
            std::ifstream test( filename.c_str() );
            if ( test.good() )
            {
#ifndef NESTIO
              std::string msg = String::compose(
                "The device file '%1' exists already and will not be overwritten. "
                "Please change data_path, data_prefix or label, or set /overwrite_files "
                "to true in the root node.",
                filename );
              LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
#endif // NESTIO
              throw IOError();
            }
            else
              test.close();

            // file does not exist, so we can open
            file.open( filename.c_str() );
          }

          if ( P_.fbuffer_size_ != P_.fbuffer_size_old_ )
          {
            if ( P_.fbuffer_size_ == 0 )
              file.rdbuf()->pubsetbuf( 0, 0 );
            else
            {
              std::vector< char >* buffer = new std::vector< char >( P_.fbuffer_size_ );
              file.rdbuf()->pubsetbuf(
                reinterpret_cast< char* >( &buffer[ 0 ] ), P_.fbuffer_size_ );
            }

            P_.fbuffer_size_old_ = P_.fbuffer_size_;
          }
        }

        if ( !file.good() )
        {
#ifndef NESTIO
          std::string msg = String::compose(
            "I/O error while opening file '%1'. "
            "This may be caused by too many open files in networks "
            "with many recording devices and threads.",
            filename );
          LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
#endif // NESTIO

          if ( file.is_open() )
            file.close();
          filename.clear();
          throw IOError();
        }

        /* Set formatting */
        file << std::fixed;
        file << std::setprecision( P_.precision_ );

        if ( P_.fbuffer_size_ != P_.fbuffer_size_old_ )
        {
#ifndef NESTIO
          std::string msg = String::compose(
            "Cannot set file buffer size, as the file is already "
            "openeded with a buffer size of %1. Please close the "
            "file first.",
            P_.fbuffer_size_old_ );
          LOG( M_ERROR, "RecordingDevice::calibrate()", msg );
#endif // NESTIO
          throw IOError();
        }
      }
    }
    catch ( std::exception& e )
    {
#pragma omp critical
      if ( !we )
        we = new WrappedThreadException( e );
    }
  } // parallel

  // check if any exceptions have been raised
  if ( we )
  {
    WrappedThreadException wec( *we );
    delete we;
    throw wec;
  }
}

void
nest::ASCIILogger::finalize()
{
  // we need to delay the throwing of exceptions to the end of the parallel section
  WrappedThreadException* we = NULL;

#pragma omp parallel
  {
    thread t = kernel().vp_manager.get_thread_id();
    thread task = kernel().vp_manager.thread_to_vp( t );

    try
    {
      // guarantee that we have initialized the inner map
      assert( ( files_.find( task ) != files_.end() ) && "initialize() has not been called" );

      // extract the inner map (containing the registered devices) for the specific VP
      typedef file_map::mapped_type inner_map;
      inner_map inner = files_[ task ];
      // iterate over registed devices and their corresponding file streams
      for ( inner_map::iterator jj = inner.begin(); jj != inner.end(); ++jj )
      {
        //int gid = jj->first;
        std::ofstream& file = *( jj->second.second );
        RecordingDevice& device = *( jj->second.first );

        if ( file.is_open() )
        {
          if ( P_.close_after_simulate_ )
          {
            file.close();
          }
          else
          {
            if ( P_.flush_after_simulate_ )
              file.flush();

            // FIXME: can this ever happen / does the message make sense?
            if ( !file.good() )
            {
#ifndef NESTIO
              std::string msg =
                String::compose( "I/O error while closing file '%1'", device.get_filename() );
              LOG( M_ERROR, "RecordingDevice::finalize()", msg );
#endif // NESTIO

              throw IOError();
            }
          }
        }
      }
    }
    catch ( std::exception& e )
    {
#pragma omp critical
      if ( !we )
        we = new WrappedThreadException( e );
    }
  } // parallel

  // check if any exceptions have been raised
  if ( we )
  {
    WrappedThreadException wec( *we );
    delete we;
    throw wec;
  }
}

void
nest::ASCIILogger::synchronize()
{
}

void
nest::ASCIILogger::write( const RecordingDevice& device, const Event& event )
{
  int vp = device.get_vp();
  int id = device.get_gid();

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ vp ][ id ].second );
  file << sender << "\t" << stamp.get_ms() - offset << "\n";
}

void
nest::ASCIILogger::write( const RecordingDevice& device,
  const Event& event,
  const std::vector< double_t >& values )
{
  int vp = device.get_vp();
  int id = device.get_gid();

  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  std::ofstream& file = *( files_[ vp ][ id ].second );
  file << sender << "\t" << stamp.get_ms() - offset;

  for ( std::vector< double_t >::const_iterator val = values.begin(); val != values.end(); ++val )
  {
    file << "\t" << *val;
  }

  file << "\n";
}

const std::string
nest::ASCIILogger::build_filename_( const RecordingDevice& device ) const
{
  // number of digits in number of virtual processes
  const int vpdigits = static_cast< int >( std::floor( std::log10( static_cast< float >(
                                             kernel().vp_manager.get_num_virtual_processes() ) ) )
    + 1 );
  const int gidigits = static_cast< int >(
    std::floor( std::log10( static_cast< float >( kernel().node_manager.size() ) ) ) + 1 );

  std::ostringstream basename;
  const std::string& path = kernel().io_manager.get_data_path();
  if ( !path.empty() )
    basename << path << '/';
  basename << kernel().io_manager.get_data_prefix();

  const std::string& label = device.get_label();
  if ( !label.empty() )
    basename << label;
  else
    basename << device.get_name();

  int vp = device.get_vp();
  int gid = device.get_gid();

  basename << "-" << std::setfill( '0' ) << std::setw( gidigits ) << gid << "-"
           << std::setfill( '0' ) << std::setw( vpdigits ) << vp;

  return basename.str() + '.' + P_.file_ext_;
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

nest::ASCIILogger::Parameters_::Parameters_()
  : precision_( 3 )
  , file_ext_( "dat" )
  , fbuffer_size_( BUFSIZ ) // default buffer size as defined in <cstdio>
  , close_after_simulate_( false )
  , flush_after_simulate_( true )
{
}

void
nest::ASCIILogger::Parameters_::get( const ASCIILogger& , DictionaryDatum& d ) const
{
  ( *d )[ names::precision ] = precision_;
  ( *d )[ names::file_extension ] = file_ext_;
  ( *d )[ names::fbuffer_size ] = fbuffer_size_;
  ( *d )[ names::close_after_simulate ] = close_after_simulate_;
  ( *d )[ names::flush_after_simulate ] = flush_after_simulate_;
}

void
nest::ASCIILogger::Parameters_::set( const ASCIILogger& , const DictionaryDatum& d )
{
  updateValue< long >( d, names::precision, precision_ );
  updateValue< std::string >( d, names::file_extension, file_ext_ );
  updateValue< bool >( d, names::close_after_simulate, close_after_simulate_ );
  updateValue< bool >( d, names::flush_after_simulate, flush_after_simulate_ );

  long fbuffer_size;
  if ( updateValue< long >( d, names::fbuffer_size, fbuffer_size ) )
  {
    if ( fbuffer_size < 0 )
      throw BadProperty( "/fbuffer_size must be <= 0" );
    else
    {
      fbuffer_size_old_ = fbuffer_size_;
      fbuffer_size_ = fbuffer_size;
    }
  }
}

void
nest::ASCIILogger::set_status( const DictionaryDatum& d )
{
  d->info(std::cout);
  
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( *this, d );  // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}
