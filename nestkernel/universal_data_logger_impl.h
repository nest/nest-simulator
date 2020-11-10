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

#include "universal_data_logger.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "nest_time.h"
#include "node.h"

template < typename HostNode >
nest::DynamicUniversalDataLogger< HostNode >::DynamicUniversalDataLogger( HostNode& host )
  : host_( host )
  , data_loggers_()
{
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::reset()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->reset();
  }
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::init()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->init();
  }
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::record_data( long step )
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->record_data( host_, step );
  }
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::handle( const DataLoggingRequest& dlr )
{
  const rport rport = dlr.get_rport();
  assert( rport >= 1 );
  assert( static_cast< size_t >( rport ) <= data_loggers_.size() );
  data_loggers_[ rport - 1 ].handle( host_, dlr );
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::DataLogger_::reset()
{
  data_.clear();
  next_rec_step_ = -1; // flag as uninitialized
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::DataLogger_::init()
{
  if ( num_vars_ < 1 )
  {
    return;
  } // not recording anything

  // Next recording step is in current slice or beyond, indicates that
  // buffer is properly initialized.
  if ( next_rec_step_ >= kernel().simulation_manager.get_slice_origin().get_steps() )
  {
    return;
  }

  // If we get here, the buffer has either never been initialized or has
  // been dormant during a period when the host node was frozen. We then
  // (re-)initialize.
  data_.clear();

  // store recording time in steps
  rec_int_steps_ = recording_interval_.get_steps();

  // set next recording step to first multiple of rec_int_steps_
  // beyond current time, shifted one to left, since rec_step marks
  // left of update intervals, and we want time stamps at right end of
  // update interval to be multiples of recording interval. Need to add
  // +1 because the division result is rounded down.
  next_rec_step_ = ( kernel().simulation_manager.get_time().get_steps() / rec_int_steps_ + 1 ) * rec_int_steps_ - 1;

  // If offset is not 0, adjust next recording step to account for it by
  // first setting next recording step to be offset and then iterating until
  // the variable is greater than current simulation time.
  if ( recording_offset_.get_steps() != 0 )
  {
    next_rec_step_ = recording_offset_.get_steps() - 1; // shifted one to left
    while ( next_rec_step_ <= kernel().simulation_manager.get_time().get_steps() )
    {
      next_rec_step_ += rec_int_steps_;
    }
  }

  // number of data points per slice
  const long recs_per_slice = static_cast< long >(
    std::ceil( kernel().connection_manager.get_min_delay() / static_cast< double >( rec_int_steps_ ) ) );

  data_.resize( 2, DataLoggingReply::Container( recs_per_slice, DataLoggingReply::Item( num_vars_ ) ) );

  next_rec_.resize( 2 );               // just for safety's sake
  next_rec_[ 0 ] = next_rec_[ 1 ] = 0; // start at beginning of buffer
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::DataLogger_::record_data( const HostNode& host, long step )
{
  if ( num_vars_ < 1 || step < next_rec_step_ )
  {
    return;
  }

  const size_t wt = kernel().event_delivery_manager.write_toggle();

  assert( wt < next_rec_.size() );
  assert( wt < data_.size() );

  /* The following assertion may fire if the multimeter connected to
     this logger is frozen. In that case, handle() is not called and
     next_rec_[wt] never reset. The assert() prevents error propagation.
     This is not an exception, since I consider the chance of users
     freezing multimeters very slim.
     See #464 for details.
   */
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

  /* We just increment. Construction ensures that we cannot overflow,
     and read-out resets.
     Overflow is possible if the multimeter is frozen, see #464.
     In that case, the assertion above will trigger.
  */
  ++next_rec_[ wt ];
}

template < typename HostNode >
void
nest::DynamicUniversalDataLogger< HostNode >::DataLogger_::handle( HostNode& host, const DataLoggingRequest& request )
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
  const size_t rt = kernel().event_delivery_manager.read_toggle();
  assert( not data_[ rt ].empty() );

  // Check if we have valid data, i.e., data with time stamps within the
  // past time slice. This may not be the case if the node has been frozen.
  // In that case, we still reset the recording marker, to prepare for the next
  // round.
  if ( data_[ rt ][ 0 ].timestamp <= kernel().simulation_manager.get_previous_slice_origin() )
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
  kernel().event_delivery_manager.send_to_node( reply );
}

template < typename HostNode >
nest::UniversalDataLogger< HostNode >::UniversalDataLogger( HostNode& host )
  : host_( host )
  , data_loggers_()
{
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::reset()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->reset();
  }
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::init()
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->init();
  }
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::record_data( long step )
{
  for ( DLiter_ it = data_loggers_.begin(); it != data_loggers_.end(); ++it )
  {
    it->record_data( host_, step );
  }
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::handle( const DataLoggingRequest& dlr )
{
  const rport rport = dlr.get_rport();
  assert( rport >= 1 );
  assert( static_cast< size_t >( rport ) <= data_loggers_.size() );
  data_loggers_[ rport - 1 ].handle( host_, dlr );
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::DataLogger_::reset()
{
  data_.clear();
  next_rec_step_ = -1; // flag as uninitialized
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::DataLogger_::init()
{
  if ( num_vars_ < 1 )
  {
    // not recording anything
    return;
  }

  // Next recording step is in current slice or beyond, indicates that
  // buffer is properly initialized.
  if ( next_rec_step_ >= kernel().simulation_manager.get_slice_origin().get_steps() )
  {
    return;
  }

  // If we get here, the buffer has either never been initialized or has
  // been dormant during a period when the host node was frozen. We then
  // (re-)initialize.
  data_.clear();

  // store recording time in steps
  rec_int_steps_ = recording_interval_.get_steps();

  // set next recording step to first multiple of rec_int_steps_
  // beyond current time, shifted one to left, since rec_step marks
  // left of update intervals, and we want time stamps at right end of
  // update interval to be multiples of recording interval. Need to add
  // +1 because the division result is rounded down.
  next_rec_step_ = ( kernel().simulation_manager.get_time().get_steps() / rec_int_steps_ + 1 ) * rec_int_steps_ - 1;

  // If offset is not 0, adjust next recording step to account for it by
  // first setting next recording step to be offset and then iterating until
  // the variable is greater than current simulation time.
  if ( recording_offset_.get_steps() != 0 )
  {
    next_rec_step_ = recording_offset_.get_steps() - 1; // shifted one to left
    while ( next_rec_step_ <= kernel().simulation_manager.get_time().get_steps() )
    {
      next_rec_step_ += rec_int_steps_;
    }
  }

  // number of data points per slice
  const long recs_per_slice = static_cast< long >(
    std::ceil( kernel().connection_manager.get_min_delay() / static_cast< double >( rec_int_steps_ ) ) );

  data_.resize( 2, DataLoggingReply::Container( recs_per_slice, DataLoggingReply::Item( num_vars_ ) ) );

  next_rec_.resize( 2 );               // just for safety's sake
  next_rec_[ 0 ] = next_rec_[ 1 ] = 0; // start at beginning of buffer
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::DataLogger_::record_data( const HostNode& host, long step )
{
  if ( num_vars_ < 1 or step < next_rec_step_ )
  {
    return;
  }

  const size_t wt = kernel().event_delivery_manager.write_toggle();

  assert( wt < next_rec_.size() );
  assert( wt < data_.size() );

  /* The following assertion may fire if the multimeter connected to
     this logger is frozen. In that case, handle() is not called and
     next_rec_[wt] never reset. The assert() prevents error propagation.
     This is not an exception, since I consider the chance of users
     freezing multimeters very slim.
     See #464 for details.
   */
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

  /* We just increment. Construction ensures that we cannot overflow,
     and read-out resets.
     Overflow is possible if the multimeter is frozen, see #464.
     In that case, the assertion above will trigger.
  */
  ++next_rec_[ wt ];
}

template < typename HostNode >
void
nest::UniversalDataLogger< HostNode >::DataLogger_::handle( HostNode& host, const DataLoggingRequest& request )
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
  const size_t rt = kernel().event_delivery_manager.read_toggle();
  assert( not data_[ rt ].empty() );

  // Check if we have valid data, i.e., data with time stamps within the
  // past time slice. This may not be the case if the node has been frozen.
  // In that case, we still reset the recording marker, to prepare for the next
  // round.
  if ( data_[ rt ][ 0 ].timestamp <= kernel().simulation_manager.get_previous_slice_origin() )
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
  kernel().event_delivery_manager.send_to_node( reply );
}

#endif // #ifndef UNIVERSAL_DATA_LOGGER_IMPL_H
