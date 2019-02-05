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

  virtual void enroll( const RecordingDevice& device,
		       const std::vector< Name >& double_value_names,
		       const std::vector< Name >& long_value_names );

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

  virtual void write( const RecordingDevice&,
		      const Event&,
		      const std::vector< double >&,
		      const std::vector< long >& );

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
    Recordings( const std::vector< Name >& double_value_names, const std::vector< Name >& long_value_names)
      : double_value_names_( double_value_names )
      , long_value_names_( long_value_names )
      , time_in_steps_( false )
    {
      double_values_.resize( double_value_names.size() );
      long_values_.resize( long_value_names.size() );
    }

    void
    push_back( index sender, const Event& event, const std::vector< double >& double_values, const std::vector< long >& long_values )
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
    
      for ( size_t i = 0; i < double_values.size(); ++i )
      {
        double_values_[ i ].push_back( double_values[ i ] );
      }
      for ( size_t i = 0; i < long_values.size(); ++i )
      {
        long_values_[ i ].push_back( double_values[ i ] );
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

      for ( size_t i = 0; i < double_values_.size(); ++i )
      {
        initialize_property_doublevector( events, double_value_names_[ i ] );
        append_property( events,  double_value_names_[ i ], double_values_[ i ] );
      }
      for ( size_t i = 0; i < long_values_.size(); ++i )
      {
        initialize_property_intvector( events, long_value_names_[ i ] );
        append_property( events,  long_value_names_[ i ], long_values_[ i ] );
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

      for ( size_t i = 0; i < double_values_.size(); ++i )
      {
        double_values_[ i ].clear();
      }
      for ( size_t i = 0; i < long_values_.size(); ++i )
      {
        long_values_[ i ].clear();
      }
      
    }

  private:
    Recordings();

    std::vector< long > senders_; //!< sender gids of the events
    std::vector< long > targets_; //!< receiver gids of the events
    std::vector< double > times_ms_; //!< times of registered events in ms
    std::vector< long > times_steps_; //!< times of registered events in steps
    std::vector< double > times_offset_; //!< offsets of registered events if time_in_steps_
    std::vector< Name > double_value_names_;
    std::vector< Name > long_value_names_;
    std::vector< std::vector< double > > double_values_;
    std::vector< std::vector< long > > long_values_;
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
