/*
 *  universal_data_logger_impl.h
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

#ifndef UNIVERSAL_DATA_LOGGER_IMPL_H
#define UNIVERSAL_DATA_LOGGER_IMPL_H

#include "simulation_manager.h"
#include "universal_data_logger.h"

namespace nest
{

// must be defined in this file, since it is required by check_connection(),
// which typically is in h-files.
template < typename HostNode >
size_t
UniversalDataLogger< HostNode >::connect_logging_device( const DataLoggingRequest& req,
  const RecordablesMap< HostNode >& rmap )
{
  // rports are assigned consecutively, the caller may not request specific
  // rports.
  if ( req.get_rport() != 0 )
  {
    throw IllegalConnection( "Connections from multimeter to node must request rport 0." );
  }

  // ensure that we have not connected this multimeter before
  const size_t mm_node_id = req.get_sender().get_node_id();

  const auto item = std::find_if( data_loggers_.begin(),
    data_loggers_.end(),
    [ & ]( const DataLogger_& dl ) { return dl.get_mm_node_id() == mm_node_id; } );

  if ( item != data_loggers_.end() )
  {
    throw IllegalConnection( "Each multimeter can only be connected once to a given node." );
  }

  // we now know that we have no DataLogger_ for the given multimeter, so we
  // create one and push it
  data_loggers_.push_back( DataLogger_( req, rmap ) );

  // rport is index plus one, i.e., size
  return data_loggers_.size();
}

template < typename HostNode >
UniversalDataLogger< HostNode >::DataLogger_::DataLogger_( const DataLoggingRequest& req,
  const RecordablesMap< HostNode >& rmap )
  : multimeter_( req.get_sender().get_node_id() )
  , num_vars_( 0 )
  , recording_interval_( Time::neg_inf() )
  , recording_offset_( Time::ms( 0. ) )
  , rec_int_steps_( 0 )
  , next_rec_step_( -1 )
  , // flag as uninitialized
  node_access_()
  , data_()
  , next_rec_( 2, 0 )
{
  const std::vector< Name >& recvars = req.record_from();
  for ( size_t j = 0; j < recvars.size(); ++j )
  {
    // .toString() required as work-around for #339, remove when #348 is solved.
    typename RecordablesMap< HostNode >::const_iterator rec = rmap.find( recvars[ j ].toString() );

    if ( rec == rmap.end() )
    {
      // delete all access information again: the connect either succeeds
      // for all entries in recvars, or it fails, leaving the logger untouched
      node_access_.clear();
      throw IllegalConnection( "Cannot connect with unknown recordable " + recvars[ j ].toString() );
    }

    node_access_.push_back( rec->second );
  }

  num_vars_ = node_access_.size();

  if ( num_vars_ > 0 and req.get_recording_interval() < Time::step( 1 ) )
  {
    throw IllegalConnection( "Recording interval must be >= resolution." );
  }

  recording_interval_ = req.get_recording_interval();
  recording_offset_ = req.get_recording_offset();
}


// must be defined in this file, since it is required by check_connection(),
// which typically is in h-files.
template < typename HostNode >
size_t
DynamicUniversalDataLogger< HostNode >::connect_logging_device( const DataLoggingRequest& req,
  const DynamicRecordablesMap< HostNode >& rmap )
{
  // rports are assigned consecutively, the caller may not request specific
  // rports.
  if ( req.get_rport() != 0 )
  {
    throw IllegalConnection( "Connections from multimeter to node must request rport 0." );
  }

  // ensure that we have not connected this multimeter before
  const size_t mm_node_id = req.get_sender().get_node_id();
  const size_t n_loggers = data_loggers_.size();
  size_t j = 0;
  while ( j < n_loggers and data_loggers_[ j ].get_mm_node_id() != mm_node_id )
  {
    ++j;
  }
  if ( j < n_loggers )
  {
    throw IllegalConnection( "Each multimeter can only be connected once to a given node." );
  }

  // we now know that we have no DataLogger_ for the given multimeter, so we
  // create one and push it
  data_loggers_.push_back( DataLogger_( req, rmap ) );

  // rport is index plus one, i.e., size
  return data_loggers_.size();
}

template < typename HostNode >
DynamicUniversalDataLogger< HostNode >::DataLogger_::DataLogger_( const DataLoggingRequest& req,
  const DynamicRecordablesMap< HostNode >& rmap )
  : multimeter_( req.get_sender().get_node_id() )
  , num_vars_( 0 )
  , recording_interval_( Time::neg_inf() )
  , recording_offset_( Time::ms( 0. ) )
  , rec_int_steps_( 0 )
  , next_rec_step_( -1 )
  , // flag as uninitialized
  node_access_()
  , data_()
  , next_rec_( 2, 0 )
{
  const std::vector< Name >& recvars = req.record_from();
  for ( size_t j = 0; j < recvars.size(); ++j )
  {
    // .toString() required as work-around for #339, remove when #348 is solved.
    typename DynamicRecordablesMap< HostNode >::const_iterator rec = rmap.find( recvars[ j ].toString() );

    if ( rec == rmap.end() )
    {
      // delete all access information again: the connect either succeeds
      // for all entries in recvars, or it fails, leaving the logger untouched
      node_access_.clear();
      throw IllegalConnection( "Cannot connect with unknown recordable " + recvars[ j ].toString() );
    }

    node_access_.push_back( &( rec->second ) );
  }

  num_vars_ = node_access_.size();

  if ( num_vars_ > 0 and req.get_recording_interval() < Time::step( 1 ) )
  {
    throw IllegalConnection( "Recording interval must be >= resolution." );
  }

  recording_interval_ = req.get_recording_interval();
  recording_offset_ = req.get_recording_offset();
}

template < typename HostNode >
DynamicUniversalDataLogger< HostNode >::DynamicUniversalDataLogger( HostNode& host )
  : host_( host )
  , data_loggers_()
{
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::reset()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->reset();
  }
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::init()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->init();
  }
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::record_data( long step )
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->record_data( host_, step );
  }
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::handle( const DataLoggingRequest& dlr )
{
  const size_t rport = dlr.get_rport();
  assert( rport >= 1 );
  assert( static_cast< size_t >( rport ) <= data_loggers_.size() );
  data_loggers_[ rport - 1 ].handle( host_, dlr );
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::DataLogger_::reset()
{
  data_.clear();
  next_rec_step_ = -1; // flag as uninitialized
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::DataLogger_::init()
{
  if ( num_vars_ < 1 )
  {
    return;
  } // not recording anything

  // Next recording step is in current slice or beyond, indicates that
  // buffer is properly initialized.
  if ( next_rec_step_ >= kernel::manager< SimulationManager >.get_slice_origin().get_steps() )
  {
    return;
  }

  // If we get here, the buffer has either never been initialized or has been dormant
  // during a period when the host node was frozen. We then (re-)initialize.
  data_.clear();

  // store recording time in steps
  rec_int_steps_ = recording_interval_.get_steps();

  // set next recording step to first multiple of rec_int_steps_
  // beyond current time, shifted one to left, since rec_step marks
  // left of update intervals, and we want time stamps at right end of
  // update interval to be multiples of recording interval. Need to add
  // +1 because the division result is rounded down.
  next_rec_step_ =
    ( kernel::manager< SimulationManager >.get_time().get_steps() / rec_int_steps_ + 1 ) * rec_int_steps_ - 1;

  // If offset is not 0, adjust next recording step to account for it by first setting next recording
  // step to be offset and then iterating until the variable is greater than current simulation time.
  if ( recording_offset_.get_steps() != 0 )
  {
    next_rec_step_ = recording_offset_.get_steps() - 1; // shifted one to left
    while ( next_rec_step_ <= kernel::manager< SimulationManager >.get_time().get_steps() )
    {
      next_rec_step_ += rec_int_steps_;
    }
  }

  // number of data points per slice
  const long recs_per_slice = static_cast< long >(
    std::ceil( kernel::manager< ConnectionManager >.get_min_delay() / static_cast< double >( rec_int_steps_ ) ) );

  data_.resize( 2, DataLoggingReply::Container( recs_per_slice, DataLoggingReply::Item( num_vars_ ) ) );

  next_rec_.resize( 2 );               // just for safety's sake
  next_rec_[ 0 ] = next_rec_[ 1 ] = 0; // start at beginning of buffer
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::DataLogger_::record_data( const HostNode&, long step )
{
  if ( num_vars_ < 1 or step < next_rec_step_ )
  {
    return;
  }

  const size_t wt = kernel::manager< EventDeliveryManager >.write_toggle();

  assert( wt < next_rec_.size() );
  assert( wt < data_.size() );

  // The following assertion may fire if the multimeter connected to
  // this logger is frozen. In that case, handle() is not called and
  // next_rec_[wt] never reset. The assert() prevents error propagation.
  // This is not an exception, since I consider the chance of users
  // freezing multimeters very slim.
  // See #464 for details.
  assert( next_rec_[ wt ] < data_[ wt ].size() );

  DataLoggingReply::Item& dest = data_[ wt ][ next_rec_[ wt ] ];

  // set time stamp: step is left end of update interval, so add 1
  dest.timestamp = Time::step( step + 1 );

  // obtain data through access functions, calling via pointer-to-member
  for ( size_t j = 0; j < num_vars_; ++j )
  {
    dest.data[ j ] = ( *( node_access_[ j ] ) )();
  }

  next_rec_step_ += rec_int_steps_;

  // We just increment. Construction ensures that we cannot overflow,
  // and read-out resets.
  // Overflow is possible if the multimeter is frozen, see #464.
  // In that case, the assertion above will trigger.
  ++next_rec_[ wt ];
}

template < typename HostNode >
void
DynamicUniversalDataLogger< HostNode >::DataLogger_::handle( HostNode& host, const DataLoggingRequest& request )
{
  if ( num_vars_ < 1 )
  {
    return;
  } // nothing to do

  // The following assertions will fire if the user forgot to call init()
  // on the data logger.
  assert( next_rec_.size() == 2 );
  assert( data_.size() == 2 );

  // get read toggle and start and end of slice
  const size_t rt = kernel::manager< EventDeliveryManager >.read_toggle();
  assert( not data_[ rt ].empty() );

  // Check if we have valid data, i.e., data with time stamps within the
  // past time slice. This may not be the case if the node has been frozen.
  // In that case, we still reset the recording marker, to prepare for the next round.
  if ( data_[ rt ][ 0 ].timestamp <= kernel::manager< SimulationManager >.get_previous_slice_origin() )
  {
    next_rec_[ rt ] = 0;
    return;
  }

  // If recording interval and min_delay are not commensurable,
  // the last entry of data_ will not contain useful data for every
  // other slice. We mark this by time stamp -infinity.
  // Applying this mark here is less work than initializing all time stamps
  // to -infinity after each call to this function.
  if ( next_rec_[ rt ] < data_[ rt ].size() )
  {
    data_[ rt ][ next_rec_[ rt ] ].timestamp = Time::neg_inf();
  }

  // now create reply event and rigg it
  DataLoggingReply reply( data_[ rt ] );

  // "clear" data
  next_rec_[ rt ] = 0;

  reply.set_sender( host );
  reply.set_sender_node_id( host.get_node_id() );
  reply.set_receiver( request.get_sender() );
  reply.set_port( request.get_port() );

  // send it off
  kernel::manager< EventDeliveryManager >.send_to_node( reply );
}

template < typename HostNode >
UniversalDataLogger< HostNode >::UniversalDataLogger( HostNode& host )
  : host_( host )
  , data_loggers_()
{
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::reset()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->reset();
  }
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::init()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->init();
  }
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::record_data( long step )
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->record_data( host_, step );
  }
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::handle( const DataLoggingRequest& dlr )
{
  const size_t rport = dlr.get_rport();
  assert( rport >= 1 );
  assert( static_cast< size_t >( rport ) <= data_loggers_.size() );
  data_loggers_[ rport - 1 ].handle( host_, dlr );
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::DataLogger_::reset()
{
  data_.clear();
  next_rec_step_ = -1; // flag as uninitialized
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::DataLogger_::init()
{
  if ( num_vars_ < 1 )
  {
    // not recording anything
    return;
  }

  // Next recording step is in current slice or beyond, indicates that
  // buffer is properly initialized.
  if ( next_rec_step_ >= kernel::manager< SimulationManager >.get_slice_origin().get_steps() )
  {
    return;
  }

  // If we get here, the buffer has either never been initialized or has
  // been dormant during a period when the host node was frozen. We then (re-)initialize.
  data_.clear();

  // store recording time in steps
  rec_int_steps_ = recording_interval_.get_steps();

  // set next recording step to first multiple of rec_int_steps_
  // beyond current time, shifted one to left, since rec_step marks
  // left of update intervals, and we want time stamps at right end of
  // update interval to be multiples of recording interval. Need to add
  // +1 because the division result is rounded down.
  next_rec_step_ =
    ( kernel::manager< SimulationManager >.get_time().get_steps() / rec_int_steps_ + 1 ) * rec_int_steps_ - 1;

  // If offset is not 0, adjust next recording step to account for it by first setting next recording
  // step to be offset and then iterating until the variable is greater than current simulation time.
  if ( recording_offset_.get_steps() != 0 )
  {
    next_rec_step_ = recording_offset_.get_steps() - 1; // shifted one to left
    while ( next_rec_step_ <= kernel::manager< SimulationManager >.get_time().get_steps() )
    {
      next_rec_step_ += rec_int_steps_;
    }
  }

  // number of data points per slice
  const long recs_per_slice = static_cast< long >(
    std::ceil( kernel::manager< ConnectionManager >.get_min_delay() / static_cast< double >( rec_int_steps_ ) ) );

  data_.resize( 2, DataLoggingReply::Container( recs_per_slice, DataLoggingReply::Item( num_vars_ ) ) );

  next_rec_.resize( 2 );               // just for safety's sake
  next_rec_[ 0 ] = next_rec_[ 1 ] = 0; // start at beginning of buffer
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::DataLogger_::record_data( const HostNode& host, long step )
{
  if ( num_vars_ < 1 or step < next_rec_step_ )
  {
    return;
  }

  const size_t wt = kernel::manager< EventDeliveryManager >.write_toggle();

  assert( wt < next_rec_.size() );
  assert( wt < data_.size() );

  // The following assertion may fire if the multimeter connected to
  // this logger is frozen. In that case, handle() is not called and
  // next_rec_[wt] never reset. The assert() prevents error propagation.
  // This is not an exception, since I consider the chance of users
  // freezing multimeters very slim.
  // See #464 for details.
  assert( next_rec_[ wt ] < data_[ wt ].size() );

  DataLoggingReply::Item& dest = data_[ wt ][ next_rec_[ wt ] ];

  // set time stamp: step is left end of update interval, so add 1
  dest.timestamp = Time::step( step + 1 );

  // obtain data through access functions, calling via pointer-to-member
  for ( size_t j = 0; j < num_vars_; ++j )
  {
    dest.data[ j ] = ( ( host ).*( node_access_[ j ] ) )();
  }

  next_rec_step_ += rec_int_steps_;

  // We just increment. Construction ensures that we cannot overflow,
  // and read-out resets.
  // Overflow is possible if the multimeter is frozen, see #464.
  //  In that case, the assertion above will trigger.
  ++next_rec_[ wt ];
}

template < typename HostNode >
void
UniversalDataLogger< HostNode >::DataLogger_::handle( HostNode& host, const DataLoggingRequest& request )
{
  if ( num_vars_ < 1 )
  {
    // nothing to do
    return;
  }

  // The following assertions will fire if the user forgot to call init()
  // on the data logger.
  assert( next_rec_.size() == 2 );
  assert( data_.size() == 2 );

  // get read toggle and start and end of slice
  const size_t rt = kernel::manager< EventDeliveryManager >.read_toggle();
  assert( not data_[ rt ].empty() );

  // Check if we have valid data, i.e., data with time stamps within the
  // past time slice. This may not be the case if the node has been frozen.
  // In that case, we still reset the recording marker, to prepare for the next round.
  if ( data_[ rt ][ 0 ].timestamp <= kernel::manager< SimulationManager >.get_previous_slice_origin() )
  {
    next_rec_[ rt ] = 0;
    return;
  }

  // If recording interval and min_delay are not commensurable,
  // the last entry of data_ will not contain useful data for every
  // other slice. We mark this by time stamp -infinity.
  // Applying this mark here is less work than initializing all time stamps
  // to -infinity after each call to this function.
  if ( next_rec_[ rt ] < data_[ rt ].size() )
  {
    data_[ rt ][ next_rec_[ rt ] ].timestamp = Time::neg_inf();
  }

  // now create reply event and rigg it
  DataLoggingReply reply( data_[ rt ] );

  // "clear" data
  next_rec_[ rt ] = 0;

  reply.set_sender( host );
  reply.set_sender_node_id( host.get_node_id() );
  reply.set_receiver( request.get_sender() );
  reply.set_port( request.get_port() );

  // send it off
  kernel::manager< EventDeliveryManager >.send_to_node( reply );
}

}


#endif
