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
#include "network.h"
#include "dictutils.h"
#include "iostreamdatum.h"
#include "arraydatum.h"
#include "config.h"
#include "exceptions.h"
#include "sliexceptions.h"
#include <iostream> // using cerr for error message.
#include <iomanip>
#include "fdstream.h"

// nestmodule provides global access to the network, so we can
// issue warning messages. This is messy and needs cleaning up.
#include "nestmodule.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::RecordingDevice::Parameters_::Parameters_( const std::string& file_ext )
  : to_file_( false )
  , to_memory_( true )
  , time_in_steps_( false )
  , precise_times_( false )
  , fbuffer_size_( BUFSIZ ) // default buffer size as defined in <cstdio>
  , label_()
  , file_ext_( file_ext )
  , filename_()
  , close_after_simulate_( false )
  , flush_after_simulate_( true )
  , flush_records_( false )
  , close_on_reset_( true )
{
}

nest::RecordingDevice::State_::State_()
  : events_( 0 )
  , event_senders_()
  , event_times_ms_()
  , event_times_steps_()
  , event_times_offsets_()
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::RecordingDevice::Parameters_::get( const RecordingDevice& rd, DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;

  ( *d )[ names::time_in_steps ] = time_in_steps_;
  if ( rd.mode_ == RecordingDevice::SPIKE_DETECTOR )
    ( *d )[ names::precise_times ] = precise_times_;

  // We must maintain /to_file, and /to_memory, because
  // the new /record_to feature is not working in Pynest.
  ( *d )[ names::to_memory ] = to_memory_;
  ( *d )[ names::to_file ] = to_file_;

  ArrayDatum ad;
  if ( to_file_ )
    ad.push_back( LiteralDatum( names::file ) );
  if ( to_memory_ )
    ad.push_back( LiteralDatum( names::memory ) );
  ( *d )[ names::record_to ] = ad;

  ( *d )[ names::file_extension ] = file_ext_;

  ( *d )[ names::fbuffer_size ] = fbuffer_size_;

  ( *d )[ names::close_after_simulate ] = close_after_simulate_;
  ( *d )[ names::flush_after_simulate ] = flush_after_simulate_;
  ( *d )[ names::flush_records ] = flush_records_;
  ( *d )[ names::close_on_reset ] = close_on_reset_;

  if ( to_file_ && !filename_.empty() )
  {
    initialize_property_array( d, names::filenames );
    append_property( d, names::filenames, filename_ );
  }
}

void
nest::RecordingDevice::Parameters_::set( const RecordingDevice& rd,
  const Buffers_&,
  const DictionaryDatum& d )
{
  updateValue< std::string >( d, names::label, label_ );
  updateValue< bool >( d, names::time_in_steps, time_in_steps_ );
  if ( rd.mode_ == RecordingDevice::SPIKE_DETECTOR )
    updateValue< bool >( d, names::precise_times, precise_times_ );
  updateValue< std::string >( d, names::file_extension, file_ext_ );

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

  updateValue< bool >( d, names::close_after_simulate, close_after_simulate_ );
  updateValue< bool >( d, names::flush_after_simulate, flush_after_simulate_ );
  updateValue< bool >( d, names::flush_records, flush_records_ );
  updateValue< bool >( d, names::close_on_reset, close_on_reset_ );

  // In Pynest we cannot use /record_to, because we have no way to pass
  // values as LiteralDatum. Thus, we must keep the boolean flags.
  // We must have || rec_change at the end, otherwise short-circuiting may
  // mean that some flags are not read.
  bool rec_change = false;
  rec_change = updateValue< bool >( d, names::to_memory, to_memory_ ) || rec_change;
  rec_change = updateValue< bool >( d, names::to_file, to_file_ ) || rec_change;

  const bool have_record_to = d->known( names::record_to );
  if ( have_record_to )
  {
    // clear all flags
    to_file_ = to_memory_ = false;

    // check for flags present in array, could be far more elegant ...
    ArrayDatum ad = getValue< ArrayDatum >( d, names::record_to );
    for ( Token* t = ad.begin(); t != ad.end(); ++t )
      if ( *t == LiteralDatum( names::file ) || *t == Token( names::file.toString() ) )
        to_file_ = true;
      else if ( *t == LiteralDatum( names::memory ) || *t == Token( names::memory.toString() ) )
        to_memory_ = true;
      else
      {
        if ( rd.mode_ == RecordingDevice::MULTIMETER )
          throw BadProperty(
            "/to_record must be array, allowed entries: /file, /memory." );
        else
          throw BadProperty(
            "/to_record must be array, allowed entries: /file, /memory." );
      }
  }

  if ( ( rec_change || have_record_to ) && to_file_ && to_memory_ )
    NestModule::get_network().message( SLIInterpreter::M_INFO,
      "RecordingDevice::set_status",
      "Data will be recorded to file and to memory." );
}

void
nest::RecordingDevice::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  // if we already have the n_events entry, we add to it, otherwise we create it
  if ( d->known( names::n_events ) )
    ( *d )[ names::n_events ] = getValue< long >( d, names::n_events ) + events_;
  else
    ( *d )[ names::n_events ] = events_;

  DictionaryDatum dict;

  // if we already have the events dict, we use it, otherwise we create it
  if ( !d->known( names::events ) )
    dict = DictionaryDatum( new Dictionary );
  else
    dict = getValue< DictionaryDatum >( d, names::events );

  initialize_property_intvector( dict, names::senders );
  append_property( dict, names::senders, std::vector< long >( event_senders_ ) );

  if ( p.time_in_steps_ )
  {
    initialize_property_intvector( dict, names::times );
    // When not accumulating, we just add time data. When accumulating, we must add
    // time data only from one thread and ensure that time data from other threads
    // is either empty of identical to what is present.
    append_property( dict, names::times, std::vector< long >( event_times_steps_ ) );

    if ( p.precise_times_ )
    {
      initialize_property_doublevector( dict, names::offsets );
      append_property( dict, names::offsets, std::vector< double_t >( event_times_offsets_ ) );
    }
  }
  else
  {
    initialize_property_doublevector( dict, names::times );
    append_property( dict, names::times, std::vector< double_t >( event_times_ms_ ) );
  }

  ( *d )[ names::events ] = dict;
}

void
nest::RecordingDevice::State_::set( const DictionaryDatum& d )
{
  long_t ne = 0;
  if ( updateValue< long_t >( d, names::n_events, ne ) )
  {
    if ( ne == 0 )
      events_ = 0;
    else
      throw BadProperty( "n_events can only be set to 0." );
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for device
 * ---------------------------------------------------------------- */

nest::RecordingDevice::RecordingDevice( const Node& n,
  Mode mode,
  const std::string& file_ext )
  : Device()
  , node_( n )
  , mode_( mode )
  , P_( file_ext )
  , S_()
{
}

nest::RecordingDevice::RecordingDevice( const Node& n, const RecordingDevice& d )
  : Device( d )
  , node_( n )
  , mode_( d.mode_ )
  , P_( d.P_ )
  , S_( d.S_ )
{
}


/* ----------------------------------------------------------------
 * Device initialization functions
 * ---------------------------------------------------------------- */

void
nest::RecordingDevice::init_parameters( const RecordingDevice& pr )
{
  Device::init_parameters( pr );

  P_ = pr.P_;
  S_ = pr.S_;
}

void
nest::RecordingDevice::init_state( const RecordingDevice& pr )
{
  Device::init_state( pr );
  S_ = pr.S_;
}

void
nest::RecordingDevice::init_buffers()
{
  Device::init_buffers();

  // we only close files here, opening is left to calibrate()
  if ( P_.close_on_reset_ && B_.fs_.is_open() )
  {
    B_.fs_.close();
    P_.filename_.clear(); // filename_ only visible while file open
  }
}

void
nest::RecordingDevice::calibrate()
{
  Device::calibrate();

  Logger* logger = Node::network()->get_logger();
  logger->enroll(node_.get_vp(), *this, value_names_);
}

void
nest::RecordingDevice::finalize()
{
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::RecordingDevice::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;    // temporary copy in case of errors
  ptmp.set( *this, B_, d ); // throws if BadProperty
  State_ stmp = S_;
  stmp.set( d );

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Device::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

  if ( !P_.to_file_ && B_.fs_.is_open() )
  {
    B_.fs_.close();
    P_.filename_.clear();
  }

  if ( S_.events_ == 0 )
    S_.clear_events();
}

void nest::RecordingDevice::write( const Event& event )
{
  ++S_.events_;
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  // std::cout << "recording device sender: " << sender << std::endl;

  if ( P_.to_file_ )
  {
    Logger* logger = Node::network()->get_logger();
    logger->write( *this, event );
  }

  // storing data when recording to accumulator relies on the fact that
  // multimeter will call us only once per accumulation step
  if ( P_.to_memory_ )
    store_data_( sender, stamp, offset );
}

void nest::RecordingDevice::write( const Event& event, const std::vector< double_t >& values )
{
  ++S_.events_;
  const index sender = event.get_sender_gid();
  const Time stamp = event.get_stamp();
  const double offset = event.get_offset();

  if ( P_.to_file_ )
  {
    Logger* logger = Node::network()->get_logger();
    logger->write( *this, event, values );
  }

  // storing data when recording to accumulator relies on the fact that
  // multimeter will call us only once per accumulation step
  if ( P_.to_memory_ )
    store_data_( sender, stamp, offset );
}

void
nest::RecordingDevice::store_data_( index sender, const Time& t, double offs )
{
  S_.event_senders_.push_back( sender );

  if ( P_.time_in_steps_ )
  {
    S_.event_times_steps_.push_back( t.get_steps() );
    if ( P_.precise_times_ )
      S_.event_times_offsets_.push_back( offs );
  }
  else if ( P_.precise_times_ )
    S_.event_times_ms_.push_back( t.get_ms() - offs );
  else
    S_.event_times_ms_.push_back( t.get_ms() );
}

void
nest::RecordingDevice::State_::clear_events()
{
  events_ = 0;
  event_senders_.clear();
  event_times_ms_.clear();
  event_times_steps_.clear();
  event_times_offsets_.clear();
}

const nest::RecordingDevice::Mode&
nest::RecordingDevice::get_mode() const
{
  return mode_;
}

const nest::Node&
nest::RecordingDevice::get_node() const
{
  return node_;
}

void
nest::RecordingDevice::set_value_names( const std::vector< Name >& names )
{
	value_names_ = names;
}
