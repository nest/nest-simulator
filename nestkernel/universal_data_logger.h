/*
 *  universal_data_logger.h
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

#ifndef UNIVERSAL_DATA_LOGGER_H
#define UNIVERSAL_DATA_LOGGER_H

// C++ includes:
#include <algorithm>
#include <vector>

// Includes from nestkernel:
#include "event.h"
#include "event_delivery_manager.h"
#include "kernel_manager.h"
#include "nest_time.h"
#include "node.h"
#include "recordables_map.h"

namespace nest
{
/**
 * @note There are two data logger class templates. The UniversalDataLogger
 *       class template is connected and populated using a RecordablesMap
 *       instance. The DynamicUniversalDataLogger class templated is
 *       connected and populated using a DynamicRecordablesMap instance.
 *       Conceptually the difference is that neurons that have a
 *       RecordablesMap instance, have a static number of state variables
 *       that can be recorded, while neurons with DynamicRecordablesMap
 *       can vary the number of recordable state variables at runtime.
 *       This is the main feature of multisynapse neurons where the
 *       number of conductance channels can change at runtime, and thus
 *       the number of recordable variables also changes.
 *       The technical difference between the UniversalDataLogger and
 *       DynamicUniversalDataLogger is that the way in which the neuron's
 *       state is accessed is different. In the former class, the data is
 *       accessed using a call to a static function pointer stored in
 *       the neurons RecordablesMap. In the latter class, the data is
 *       accessed through a functor call that holds a reference to the
 *       neuron's id and the recorded state variable's index.
 */

/**
 * Universal data-logging plug-in for neuron models.
 *
 * This class provides logging of universal data such as
 * membrane potentials or conductances in a way that is
 * compatible with the DataLoggingRequest/DataLoggingReply
 * read-out mechanism. It is intended to be used by neuron models
 * that have static recordable state variables, i.e. all models except
 * multisynapse models.
 *
 * The logger must be informed about any incoming DataLoggingRequest
 * connections by calling connect_logging_device(). All incoming
 * DataLoggingRequests should then be forwarded to the logger using
 * handle().
 *
 * @note A reference to the host node is stored in the logger, for
 *       access to the state and sending events. This requires a constructor
 *       and a copy constructor for the HostNode::Buffers_, creating new
 *       logger members initialized with the new node, e.g.
 * @code
 * struct Buffers_ {
 *   Buffers_(iaf_cond_alpha&);
 *   Buffers_(const Buffers_&, iaf_cond_alpha&);
 *
 *   UniversalDataLogger<iaf_cond_alpha> logger_;
 *
 * ...
 *
 * iaf_cond_alpha::Buffers_::Buffers_(iaf_cond_alpha& n)
 * : logger_(n), ... {}
 *
 * iaf_cond_alpha::Buffers_::Buffers_(const Buffers_&, iaf_cond_alpha& n)
 * : logger_(n), ... {}
 *
 * iaf_cond_alpha::iaf_cond_alpha()
 * : ..., B_(*this) {}
 *
 * iaf_cond_alpha::iaf_cond_alpha(const iaf_cond_alpha& n)
 * : ..., B_(n.B_, *this) {}
 * @code
 *
 * @todo Could HostNode be passed as const& to handle() and record_data()?
 *
 * @addtogroup Devices
 */

template < typename HostNode >
class UniversalDataLogger
{

public:
  /**
   * Create logger.
   */
  UniversalDataLogger( HostNode& );

  /**
   * Notify data logger that the node is recorded from.
   *
   * This method must be called when a universal recording device
   * is connected to the node. It informs the data logger that
   * data actually needs to be logged. Otherwise, data is simply
   * discarded.
   *
   * @param provides information about requested data and interval
   * @param map of access functions
   * @return rport for future logging requests
   */
  size_t connect_logging_device( const DataLoggingRequest&, const RecordablesMap< HostNode >& );

  /**
   * Answer DataLoggingRequest.
   *
   * The data logger creates an event of type DataLoggingRequest
   * with a reference to the data and returns it to the requester.
   *
   * @note The request should not contain any data; it is ignored.
   *
   * @param DataLoggingRequest Request to be handled.
   */
  void handle( const DataLoggingRequest& );

  /**
   * Record data using predefined access functions.
   *
   * This function should be called once per time step at the end of the
   * time step to record data from the node to the logger.
   *
   * @param long    Time in steps at the BEGINNING of the update step, i.e.,
   *                  origin+lag, but the data is logged with time stamp
   *                  origin+lag+1, since this is the proper time of the data.
   */
  void record_data( long );

  //! Erase all existing data
  void reset();

  /**
   * Initialize logger, i.e., set up data buffers.
   *
   * Has no effect if buffer is initialized already.
   */
  void init();

private:
  /**
   * Single data logger, serving one multimeter.
   *
   * For each multimeter connected to a node, one DataLogger_ instance is
   * created. The UniversalDataLogger forwards all requests to the correct
   * DataLogger_ based on the rport of the request.
   */
  class DataLogger_
  {
  public:
    DataLogger_( const DataLoggingRequest&, const RecordablesMap< HostNode >& );
    size_t
    get_mm_node_id() const
    {
      return multimeter_;
    }
    void handle( HostNode&, const DataLoggingRequest& );
    void record_data( const HostNode&, long );
    void reset();
    void init();

  private:
    size_t multimeter_; //!< node ID of multimeter for which the logger works
    size_t num_vars_;   //!< number of variables recorded

    Time recording_interval_; //!< interval between two recordings
    Time recording_offset_;   //!< offset relative to which interval is calculated
    long rec_int_steps_;      //!< interval in steps
    long next_rec_step_;      //!< next time step at which to record

    /** Vector of pointers to member functions for data access. */
    std::vector< typename RecordablesMap< HostNode >::DataAccessFct > node_access_;

    /**
     * Buffer for data.
     *
     * The first dimension has size two, to provide for alternate
     * writing/reading using a toggle. The second dimension has
     * one entry per recording time in each time slice. Each entry
     * consists of a time stamp and one data point per recordable.
     */
    std::vector< DataLoggingReply::Container > data_;

    //! Next buffer entry to write to, with read/write toggle
    std::vector< size_t > next_rec_;
  };

  HostNode& host_; //!< node to which logger belongs

  /**
   * Data loggers, one per connected multimeter.
   *
   * Indices are rport-1.
   */
  std::vector< DataLogger_ > data_loggers_;
  typedef typename std::vector< DataLogger_ >::iterator DLiter_;

  //! Should not be copied.
  UniversalDataLogger( const UniversalDataLogger& );

  //! Should not be assigned
  UniversalDataLogger const& operator=( const UniversalDataLogger& );
};

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


/**
 * Dynamic Universal data-logging plug-in for multisynapse neuron models.
 *
 * This class provides logging of universal data such as
 * membrane potentials or conductances in a way that is
 * compatible with the DataLoggingRequest/DataLoggingReply
 * read-out mechanism. It is intended to handle logging of multisynapse
 * neuron conductances because it interacts with DynamicRecordablesMap
 * instances instead of RecordablesMap's. The difference is that
 * the DynamicalRecordablesMap stores DataAccessFunctor instances instead
 * of static function pointers. This allows the data accessor to select
 * the state variable that will be recorded at runtime, instead of at
 * compile time. This enables the DynamicUniversalDataLogger to be
 * connected to a variable number of conductance channels that are set
 * at runtime.
 *
 * The logger must be informed about any incoming DataLoggingRequest
 * connections by calling connect_logging_device(). All incoming
 * DataLoggingRequests should then be forwarded to the logger using
 * handle().
 *
 * @note A reference to the host node is stored in the logger, for
 *       access to the state and sending events. This requires a constructor
 *       and a copy constructor for the HostNode::Buffers_, creating new
 *       logger members initialized with the new node, e.g.
 * @code
 * struct Buffers_ {
 *   Buffers_(aeif_cond_beta_multisynapse&);
 *   Buffers_(const Buffers_&, aeif_cond_beta_multisynapse&);
 *
 *   DynamicUniversalDataLogger< aeif_cond_beta_multisynapse > logger_;
 *
 * ...
 *
 * aeif_cond_beta_multisynapse::Buffers_::Buffers_(aeif_cond_beta_multisynapse&
 * n)
 * : logger_(n), ... {}
 *
 * aeif_cond_beta_multisynapse::Buffers_::Buffers_(const Buffers_&,
 * aeif_cond_beta_multisynapse& n)
 * : logger_(n), ... {}
 *
 * aeif_cond_beta_multisynapse::aeif_cond_beta_multisynapse()
 * : ..., B_(*this) {}
 *
 * aeif_cond_beta_multisynapse::aeif_cond_beta_multisynapse(const
 * aeif_cond_beta_multisynapse& n)
 * : ..., B_(n.B_, *this) {}
 * @code
 *
 * @todo Could HostNode be passed as const& to handle() and record_data()?
 *
 * @addtogroup Devices
 */


template < typename HostNode >
class DynamicUniversalDataLogger
{

public:
  /**
   * Create logger.
   */
  DynamicUniversalDataLogger( HostNode& );

  /**
   * Notify data logger that the node is recorded from.
   *
   * This method must be called when a universal recording device
   * is connected to the node. It informs the data logger that
   * data actually needs to be logged. Otherwise, data is simply
   * discarded.
   *
   * @param provides information about requested data and interval
   * @param map of access functions
   * @return rport for future logging requests
   */
  size_t connect_logging_device( const DataLoggingRequest&, const DynamicRecordablesMap< HostNode >& );

  /**
   * Answer DataLoggingRequest.
   *
   * The data logger creates an event of type DataLoggingRequest
   * with a reference to the data and returns it to the requester.
   *
   * @note The request should not contain any data; it is ignored.
   *
   * @param DataLoggingRequest Request to be handled.
   */
  void handle( const DataLoggingRequest& );

  /**
   * Record data using predefined access functions.
   *
   * This function should be called once per time step at the end of the
   * time step to record data from the node to the logger.
   *
   * @param long    Time in steps at the BEGINNING of the update step, i.e.,
   *                  origin+lag, but the data is logged with time stamp
   *                  origin+lag+1, since this is the proper time of the data.
   */
  void record_data( long );

  //! Erase all existing data
  void reset();

  /**
   * Initialize logger, i.e., set up data buffers.
   *
   * Has no effect if buffer is initialized already.
   */
  void init();

private:
  /**
   * Single data logger, serving one multimeter.
   *
   * For each multimeter connected to a node, one DataLogger_ instance is
   * created. The UniversalDataLogger forwards all requests to the correct
   * DataLogger_ based on the rport of the request.
   */
  class DataLogger_
  {
  public:
    DataLogger_( const DataLoggingRequest&, const DynamicRecordablesMap< HostNode >& );
    size_t
    get_mm_node_id() const
    {
      return multimeter_;
    }
    void handle( HostNode&, const DataLoggingRequest& );
    void record_data( const HostNode&, long );
    void reset();
    void init();

  private:
    size_t multimeter_; //!< node ID of multimeter for which the logger works
    size_t num_vars_;   //!< number of variables recorded

    Time recording_interval_; //!< interval between two recordings
    Time recording_offset_;   //!< offset relative to which interval is calculated
    long rec_int_steps_;      //!< interval in steps
    long next_rec_step_;      //!< next time step at which to record

    /** Vector of pointers to member functions for data access. */
    std::vector< const typename DynamicRecordablesMap< HostNode >::DataAccessFct* > node_access_;

    /**
     * Buffer for data.
     *
     * The first dimension has size two, to provide for alternate
     * writing/reading using a toggle. The second dimension has
     * one entry per recording time in each time slice. Each entry
     * consists of a time stamp and one data point per recordable.
     */
    std::vector< DataLoggingReply::Container > data_;

    //! Next buffer entry to write to, with read/write toggle
    std::vector< size_t > next_rec_;
  };

  HostNode& host_; //!< node to which logger belongs

  /**
   * Data loggers, one per connected multimeter.
   * Indices are rport-1.
   */
  std::vector< DataLogger_ > data_loggers_;
  typedef typename std::vector< DataLogger_ >::iterator DLiter_;

  //! Should not be copied.
  DynamicUniversalDataLogger( const DynamicUniversalDataLogger& );

  //! Should not be assigned
  DynamicUniversalDataLogger const& operator=( const DynamicUniversalDataLogger& );
};


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
  if ( next_rec_step_ >= kernel::manager< SimulationManager >().get_slice_origin().get_steps() )
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
    ( kernel::manager< SimulationManager >().get_time().get_steps() / rec_int_steps_ + 1 ) * rec_int_steps_ - 1;

  // If offset is not 0, adjust next recording step to account for it by first setting next recording
  // step to be offset and then iterating until the variable is greater than current simulation time.
  if ( recording_offset_.get_steps() != 0 )
  {
    next_rec_step_ = recording_offset_.get_steps() - 1; // shifted one to left
    while ( next_rec_step_ <= kernel::manager< SimulationManager >().get_time().get_steps() )
    {
      next_rec_step_ += rec_int_steps_;
    }
  }

  // number of data points per slice
  const long recs_per_slice = static_cast< long >(
    std::ceil( kernel::manager< ConnectionManager >().get_min_delay() / static_cast< double >( rec_int_steps_ ) ) );

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

  const size_t wt = kernel::manager< EventDeliveryManager >().write_toggle();

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
  const size_t rt = kernel::manager< EventDeliveryManager >().read_toggle();
  assert( not data_[ rt ].empty() );

  // Check if we have valid data, i.e., data with time stamps within the
  // past time slice. This may not be the case if the node has been frozen.
  // In that case, we still reset the recording marker, to prepare for the next round.
  if ( data_[ rt ][ 0 ].timestamp <= kernel::manager< SimulationManager >().get_previous_slice_origin() )
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
  kernel::manager< EventDeliveryManager >().send_to_node( reply );
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
  if ( next_rec_step_ >= kernel::manager< SimulationManager >().get_slice_origin().get_steps() )
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
    ( kernel::manager< SimulationManager >().get_time().get_steps() / rec_int_steps_ + 1 ) * rec_int_steps_ - 1;

  // If offset is not 0, adjust next recording step to account for it by first setting next recording
  // step to be offset and then iterating until the variable is greater than current simulation time.
  if ( recording_offset_.get_steps() != 0 )
  {
    next_rec_step_ = recording_offset_.get_steps() - 1; // shifted one to left
    while ( next_rec_step_ <= kernel::manager< SimulationManager >().get_time().get_steps() )
    {
      next_rec_step_ += rec_int_steps_;
    }
  }

  // number of data points per slice
  const long recs_per_slice = static_cast< long >(
    std::ceil( kernel::manager< ConnectionManager >().get_min_delay() / static_cast< double >( rec_int_steps_ ) ) );

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

  const size_t wt = kernel::manager< EventDeliveryManager >().write_toggle();

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
  const size_t rt = kernel::manager< EventDeliveryManager >().read_toggle();
  assert( not data_[ rt ].empty() );

  // Check if we have valid data, i.e., data with time stamps within the
  // past time slice. This may not be the case if the node has been frozen.
  // In that case, we still reset the recording marker, to prepare for the next round.
  if ( data_[ rt ][ 0 ].timestamp <= kernel::manager< SimulationManager >().get_previous_slice_origin() )
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
  kernel::manager< EventDeliveryManager >().send_to_node( reply );
}

} // namespace nest

#endif /* #ifndef UNIVERSAL_DATA_LOGGER_H */
