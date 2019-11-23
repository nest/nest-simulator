/*
 *  multimeter.cpp
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

#include "multimeter.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"

// Includes from libnestutil:
#include "dict_util.h"

namespace nest
{
multimeter::multimeter()
  : RecordingDevice()
  , P_()
  , B_()
{
}

multimeter::multimeter( const multimeter& n )
  : RecordingDevice( n )
  , P_( n.P_ )
  , B_()
{
}

port
multimeter::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  DataLoggingRequest e( P_.interval_, P_.offset_, P_.record_from_ );
  e.set_sender( *this );
  port p = target.handles_test_event( e, receptor_type );
  if ( p != invalid_port_ and not is_model_prototype() )
  {
    B_.has_targets_ = true;
  }
  return p;
}

nest::multimeter::Parameters_::Parameters_()
  : interval_( Time::ms( 1.0 ) )
  , offset_( Time::ms( 0. ) )
  , record_from_()
{
}

nest::multimeter::Parameters_::Parameters_( const Parameters_& p )
  : interval_( p.interval_ )
  , offset_( p.offset_ )
  , record_from_( p.record_from_ )
{
  interval_.calibrate();
}

nest::multimeter::Buffers_::Buffers_()
  : has_targets_( false )
{
}

void
nest::multimeter::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::interval ] = interval_.get_ms();
  ( *d )[ names::offset ] = offset_.get_ms();
  ArrayDatum ad;
  for ( size_t j = 0; j < record_from_.size(); ++j )
  {
    ad.push_back( LiteralDatum( record_from_[ j ] ) );
  }
  ( *d )[ names::record_from ] = ad;
}

void
nest::multimeter::Parameters_::set( const DictionaryDatum& d, const Buffers_& b, Node* node )
{
  if ( b.has_targets_
    && ( d->known( names::interval ) || d->known( names::offset ) || d->known( names::record_from ) ) )
  {
    throw BadProperty(
      "The recording interval, the interval offset and the list of properties "
      "to record cannot be changed after the multimeter has been connected "
      "to nodes." );
  }

  double v;
  if ( updateValueParam< double >( d, names::interval, v, node ) )
  {
    if ( Time( Time::ms( v ) ) < Time::get_resolution() )
    {
      throw BadProperty(
        "The sampling interval must be at least as long "
        "as the simulation resolution." );
    }

    // see if we can represent interval as multiple of step
    interval_ = Time::step( Time( Time::ms( v ) ).get_steps() );
    if ( not interval_.is_multiple_of( Time::get_resolution() ) )
    {
      throw BadProperty(
        "The sampling interval must be a multiple of "
        "the simulation resolution" );
    }
  }

  if ( updateValueParam< double >( d, names::offset, v, node ) )
  {
    // if offset is different from the default value (0), it must be at least
    // as large as the resolution
    if ( v != 0 && Time( Time::ms( v ) ) < Time::get_resolution() )
    {
      throw BadProperty(
        "The offset for the sampling interval must be at least as long as the "
        "simulation resolution." );
    }

    // see if we can represent offset as multiple of step
    offset_ = Time::step( Time( Time::ms( v ) ).get_steps() );
    if ( not offset_.is_multiple_of( Time::get_resolution() ) )
    {
      throw BadProperty(
        "The offset for the sampling interval must be a multiple of the "
        "simulation resolution" );
    }
  }

  // extract data
  if ( d->known( names::record_from ) )
  {
    record_from_.clear();

    ArrayDatum ad = getValue< ArrayDatum >( d, names::record_from );
    for ( Token* t = ad.begin(); t != ad.end(); ++t )
    {
      record_from_.push_back( Name( getValue< std::string >( *t ) ) );
    }
  }
}

void
multimeter::calibrate()
{
  RecordingDevice::calibrate( P_.record_from_, RecordingBackend::NO_LONG_VALUE_NAMES );
}

void
multimeter::update( Time const& origin, const long from, const long )
{
  /* There is nothing to request during the first time slice.
     For each subsequent slice, we collect all data generated during the
     previous slice if we are called at the beginning of the slice. Otherwise,
     we do nothing.
   */
  if ( origin.get_steps() == 0 || from != 0 )
  {
    return;
  }

  // We send a request to each of our targets.
  // The target then immediately returns a DataLoggingReply event,
  // which is caught by multimeter::handle(), which in turn
  // ensures that the event is recorded.
  // handle() has access to request_, so it knows what we asked for.
  //
  // Note that not all nodes receiving the request will necessarily answer.
  DataLoggingRequest req;
  kernel().event_delivery_manager.send( *this, req );
}

void
multimeter::handle( DataLoggingReply& reply )
{
  // easy access to relevant information
  DataLoggingReply::Container const& info = reply.get_info();

  // count records that have been skipped during inactivity
  size_t inactive_skipped = 0;

  // record all data, time point by time point
  for ( size_t j = 0; j < info.size(); ++j )
  {
    if ( not info[ j ].timestamp.is_finite() )
    {
      break;
    }

    if ( not is_active( info[ j ].timestamp ) )
    {
      ++inactive_skipped;
      continue;
    }

    reply.set_stamp( info[ j ].timestamp );
    // const index sender = reply.get_sender_node_id();
    // const Time stamp = reply.get_stamp();
    // const double offset = reply.get_offset();

    write( reply, info[ j ].data, RecordingBackend::NO_LONG_VALUES );
  }
}

RecordingDevice::Type
multimeter::get_type() const
{
  return RecordingDevice::MULTIMETER;
}

} // namespace nest
