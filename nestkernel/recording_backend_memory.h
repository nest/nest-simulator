/*
 *  recording_backend_memory.h
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

#ifndef RECORDING_BACKEND_MEMORY_H
#define RECORDING_BACKEND_MEMORY_H

// Includes from sli:
#include "arraydatum.h"

#include "recording_backend.h"

namespace nest
{

/**
 * Memory specialization of the RecordingBackend interface.
 *
 * Recorded data is stored in memory on a per-device-per-thread
 * basis. Setting the /num_events in the status dictionary of an
 * individual device will wipe the data for that device from memory.
 *
 * RecordingBackendMemory maintains a data structure mapping the data
 * vectors to every recording device instance on every thread. The
 * basic data structure is initialized during the initialize() call
 * and closed in finalize(). The concrete data vectors are added to
 * the basic data structure during the call to enroll(), when the
 * exact fields are known.
 */
class RecordingBackendMemory : public RecordingBackend
{
public:
  /**
   * RecordingBackendMemory constructor.
   * The actual setup is done in initialize().
   */
  RecordingBackendMemory()
    : data_()
  {
  }

  /**
   * RecordingBackendMemory descructor.
   */
  ~RecordingBackendMemory() throw();

  /**
   * Functions called by all instantiated recording devices to register
   * themselves with their metadata.
   */
  void enroll( RecordingDevice& device );
  void enroll( RecordingDevice& device,
    const std::vector< Name >& value_names );

  /**
   * Finalize the RecordingBackendMemory after the simulation has finished.
   */
  void finalize();

  void get_device_status_( const RecordingDevice& device,
    DictionaryDatum& ) const;

  /**
   * Trivial synchronization function. The RecordingBackendMemory does
   * not need explicit synchronization after each time step.
   */
  void synchronize();

  /**
   * Clear the recorded data for the given RecordingDevice.
   */
  void clear( const RecordingDevice& );

  /**
   * Functions to write data to memory.
   */
  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& values );

protected:
  /**
   * Initialize the RecordingBackendMemory during simulation preparation.
   */
  void initialize_();


private:
  class Recordings
  {
  public:
    Recordings( const std::vector< Name >& extra_data_names )
      : extra_data_names_( extra_data_names )
    {
    }

    void
    push_back( index sender, double time )
    {
      senders_.push_back( sender );
      times_.push_back( time );
    }

    void
    push_back( index sender, double time, const std::vector< double >& values )
    {
      push_back( sender, time );
      for ( size_t i = 0; i < values.size(); ++i )
      {
        extra_data_[ i ].push_back( values[ i ] );
      }
    }

    void
    get_status( DictionaryDatum& d ) const
    {
      DictionaryDatum dd( new Dictionary() );
      ( *dd )[ names::senders ] =
        IntVectorDatum( new std::vector< long >( senders_ ) );
      ( *dd )[ names::times ] =
        DoubleVectorDatum( new std::vector< double >( times_ ) );
      for ( size_t i = 0; i < extra_data_.size(); ++i )
      {
        ( *dd )[ extra_data_names_[ i ] ] =
          DoubleVectorDatum( new std::vector< double >( extra_data_[ i ] ) );
      }
      ( *d )[ names::events ] = dd;
    }

    void
    clear()
    {
      senders_.clear();
      times_.clear();
      for ( size_t i = 0; i < extra_data_.size(); ++i )
      {
        extra_data_[ i ].clear();
      }
    }

  private:
    Recordings();

    std::vector< long > senders_; //!< the gids of the senders of the events
    std::vector< double > times_; //!< the times of the registered events
    std::vector< std::vector< double > > extra_data_;
    std::vector< Name > extra_data_names_;
  };

  /**
   * A map for the data. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its recordings.
  */
  typedef std::vector< std::map< size_t, Recordings* > > data_map;
  data_map data_;
};

} // namespace

#endif // RECORDING_BACKEND_MEMORY_H
