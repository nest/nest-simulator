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

namespace nest
{
Multimeter::Multimeter()
  : DeviceNode()
  , device_( *this, RecordingDevice::MULTIMETER, "dat", true, true )
  , P_()
  , S_()
  , B_()
  , V_()
{
}

Multimeter::Multimeter( const Multimeter& n )
  : DeviceNode( n )
  , device_( *this, n.device_ )
  , P_( n.P_ )
  , S_()
  , B_()
  , V_()
{
}

port
Multimeter::send_test_event( Node& target, rport receptor_type, synindex, bool )
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

nest::Multimeter::Parameters_::Parameters_()
  : interval_( Time::ms( 1.0 ) )
  , offset_( Time::ms( 0. ) )
  , record_from_()
{
}

nest::Multimeter::Parameters_::Parameters_( const Parameters_& p )
  : interval_( p.interval_ )
  , offset_( p.offset_ )
  , record_from_( p.record_from_ )
{
  interval_.calibrate();
}

nest::Multimeter::Buffers_::Buffers_()
  : has_targets_( false )
{
}

void
nest::Multimeter::Parameters_::get( DictionaryDatum& d ) const
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
nest::Multimeter::Parameters_::set( const DictionaryDatum& d, const Buffers_& b )
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
  if ( updateValue< double >( d, names::interval, v ) )
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

  if ( updateValue< double >( d, names::offset, v ) )
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
Multimeter::init_state_( const Node& np )
{
  const Multimeter& asd = dynamic_cast< const Multimeter& >( np );
  device_.init_state( asd.device_ );
  S_.data_.clear();
}

void
Multimeter::init_buffers_()
{
  device_.init_buffers();
}

void
Multimeter::calibrate()
{
  device_.calibrate();
  V_.new_request_ = false;
  V_.current_request_data_start_ = 0;
}

void
Multimeter::post_run_cleanup()
{
  device_.post_run_cleanup();
}

void
Multimeter::finalize()
{
  device_.finalize();
}

void
Multimeter::update( Time const& origin, const long from, const long )
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
  // Provided we are recording anything, V_.new_request_ is set to true. This
  // informs handle() that the first incoming DataLoggingReply is for a new time
  // slice, so that the data from that first Reply must be pushed back; all
  // following Reply data is then added.
  //
  // Note that not all nodes receiving the request will necessarily answer.
  V_.new_request_ = B_.has_targets_ && not P_.record_from_.empty(); // no targets, no request
  DataLoggingRequest req;
  kernel().event_delivery_manager.send( *this, req );
}

void
Multimeter::handle( DataLoggingReply& reply )
{
  // easy access to relevant information
  DataLoggingReply::Container const& info = reply.get_info();

  // If this is the first Reply arriving, we need to mark the beginning of the
  // data for this round of replies
  if ( V_.new_request_ )
  {
    V_.current_request_data_start_ = S_.data_.size();
  }

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

    // store stamp for current data set in event for logging
    reply.set_stamp( info[ j ].timestamp );

    // record sender and time information; in accumulator mode only for first
    // Reply in slice
    if ( not device_.to_accumulator() || V_.new_request_ )
    {
      device_.record_event( reply, false ); // false: more data to come
    }

    if ( not device_.to_accumulator() )
    {
      // "print" actual data, but not in accumulator mode
      print_value_( info[ j ].data );

      if ( device_.to_memory() )
      {
        S_.data_.push_back( info[ j ].data );
      }
    }
    else
    {
      if ( V_.new_request_ ) // first reply in slice, push back to create new
                             // time points
      {
        S_.data_.push_back( info[ j ].data );
      }
      else
      { // add data; offset j from current_request_data_start_, but inactive
        // skipped entries subtracted
        assert( j >= inactive_skipped );
        assert( V_.current_request_data_start_ + j - inactive_skipped < S_.data_.size() );
        assert( S_.data_[ V_.current_request_data_start_ + j - inactive_skipped ].size() == info[ j ].data.size() );

        for ( size_t k = 0; k < info[ j ].data.size(); ++k )
        {
          S_.data_[ V_.current_request_data_start_ + j - inactive_skipped ][ k ] += info[ j ].data[ k ];
        }
      }
    }
  }

  // correct either we are done with the first reply or any later one
  V_.new_request_ = false;
}

void
Multimeter::print_value_( const std::vector< double >& values )
{
  if ( values.size() < 1 )
  {
    return;
  }

  for ( size_t j = 0; j < values.size() - 1; ++j )
  {
    device_.print_value( values[ j ], false );
  }

  device_.print_value( values[ values.size() - 1 ] );
}


void
Multimeter::add_data_( DictionaryDatum& d ) const
{
  // re-organize data into one vector per recorded variable
  for ( size_t v = 0; v < P_.record_from_.size(); ++v )
  {
    std::vector< double > dv( S_.data_.size() );
    for ( size_t t = 0; t < S_.data_.size(); ++t )
    {
      assert( v < S_.data_[ t ].size() );
      dv[ t ] = S_.data_[ t ][ v ];
    }
    initialize_property_doublevector( d, P_.record_from_[ v ] );
    if ( device_.to_accumulator() && not dv.empty() )
    {
      accumulate_property( d, P_.record_from_[ v ], dv );
    }
    else
    {
      append_property( d, P_.record_from_[ v ], dv );
    }
  }
}

bool
Multimeter::is_active( Time const& T ) const
{
  const long stamp = T.get_steps();

  return device_.get_t_min_() < stamp && stamp <= device_.get_t_max_();
}
}
