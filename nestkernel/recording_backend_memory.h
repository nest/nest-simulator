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
 * basis. Setting the /n_events in the status dictionary of an
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
  RecordingBackendMemory();

  /**
   * RecordingBackendMemory descructor.
   */
  ~RecordingBackendMemory() throw();

  /**
   * Functions called by all instantiated recording devices to register
   * themselves with their metadata.
   */
  void enroll( const RecordingDevice& device );
  void enroll( const RecordingDevice& device,
    const std::vector< Name >& value_names );

  /**
   * Finalize the RecordingBackendMemory after the simulation has finished.
   */
  void finalize();

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

  /**
   * Initialize the RecordingBackendMemory during simulation preparation.
   */
  void initialize();

  void get_device_status( const RecordingDevice& device,
			  DictionaryDatum& ) const;

  void set_device_status( const RecordingDevice& device,
			  const DictionaryDatum& );

private:
  class Recordings
  {
  public:
    Recordings( const std::vector< Name >& extra_data_names)
      : extra_data_names_( extra_data_names )
      , time_in_steps_( false )
    {
      extra_data_.resize( extra_data_names.size() );
    }

    void
    push_back( index sender, const Event& event )
    {
      const Time stamp = event.get_stamp();
      const double offset = event.get_offset();

      senders_.push_back( sender );

      if ( record_targets_ )
      {
        targets_.push_back( event.get_receiver_gid() );
      }
      
      if ( time_in_steps_ )
      {
        times_steps_.push_back( stamp.get_steps() );
        times_offset_.push_back( offset );
      }
      else
      {
        times_ms_.push_back( stamp.get_ms() - offset );
      }
    }

    void
    push_back( index sender, const Event& event, const std::vector< double >& values )
    {
      push_back( sender, event );
      for ( size_t i = 0; i < values.size(); ++i )
      {
        extra_data_[ i ].push_back( values[ i ] );
      }
    }

    void
    get_status( DictionaryDatum& d ) const
    {
      DictionaryDatum events;

      if ( not d->known( names::events ) )
      {
	events = DictionaryDatum( new Dictionary );
	( *d )[ names::events ] = events;
      }
      else
      {
	events = getValue< DictionaryDatum >( d, names::events );
      }

      initialize_property_intvector( events, names::senders );
      append_property( events, names::senders, senders_ );

      if ( record_targets_ )
      {
        initialize_property_intvector( events, names::targets );
        append_property( events,  names::targets, targets_ );
      }	  

      if ( time_in_steps_ )
      {
        initialize_property_intvector( events, names::times );
        append_property( events,  names::times, times_steps_ );

        initialize_property_doublevector( events, names::offsets );
        append_property( events,  names::offsets, times_offset_ );
      }
      else
      {
        initialize_property_doublevector( events, names::times );
        append_property( events,  names::times, times_ms_ );
      }

      for ( size_t i = 0; i < extra_data_.size(); ++i )
      {
        initialize_property_doublevector( events, extra_data_names_[ i ] );
        append_property( events,  extra_data_names_[ i ], extra_data_ [ i ] );
      }
    }

    void
    set_time_in_steps( bool time_in_steps )
    {
      time_in_steps_ = time_in_steps;
    }

    void
    set_record_targets( bool record_targets )
    {
      record_targets_ = record_targets;
    }
    
    void
    clear()
    {
      senders_.clear();
      targets_.clear();
      times_ms_.clear();
      times_steps_.clear();
      times_offset_.clear();

      for ( size_t i = 0; i < extra_data_.size(); ++i )
      {
        extra_data_[ i ].clear();
      }
    }

  private:
    Recordings();

    std::vector< long > senders_; //!< sender gids of the events
    std::vector< long > targets_; //!< receiver gids of the events
    std::vector< double > times_ms_; //!< times of registered events in ms
    std::vector< long > times_steps_; //!< times of registered events in steps
    std::vector< double > times_offset_; //!< offsets of registered events if time_in_steps_
    std::vector< std::vector< double > > extra_data_;
    std::vector< Name > extra_data_names_;
    bool time_in_steps_;
    bool record_targets_;
  };

  /**
   * A map for the data. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its recordings.
  */
  typedef std::vector< std::map< size_t, Recordings* > > data_map;
  data_map data_;

  void delete_data_();
};

} // namespace

#endif // RECORDING_BACKEND_MEMORY_H
