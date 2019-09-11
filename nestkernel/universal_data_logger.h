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
#include <vector>

// Includes from nestkernel:
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"
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
 * nest::iaf_cond_alpha::Buffers_::Buffers_(iaf_cond_alpha& n)
 * : logger_(n), ... {}
 *
 * nest::iaf_cond_alpha::Buffers_::Buffers_(const Buffers_&, iaf_cond_alpha& n)
 * : logger_(n), ... {}
 *
 * nest::iaf_cond_alpha::iaf_cond_alpha()
 * : ..., B_(*this) {}
 *
 * nest::iaf_cond_alpha::iaf_cond_alpha(const iaf_cond_alpha& n)
 * : ..., B_(n.B_, *this) {}
 * @code
 *
 * @todo Could HostNode be passed as const& to handle() and record_data()?
 *
 * @note To avoid inclusion problems and code-bloat, the class
 *       interface is defined in this file, while most of the
 *       implementation is in the companion universal_data_logger_impl.h.
 *       As a consequence, calls to UniversalDataLogger members should
 *       only come from cpp files---do not inline them.
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
  port connect_logging_device( const DataLoggingRequest&, const RecordablesMap< HostNode >& );

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
   * Has no effect if buffer is initialized already.
   */
  void init();

private:
  /**
   * Single data logger, serving one multimeter.
   * For each multimeter connected to a node, one DataLogger_ instance is
   * created. The UniversalDataLogger forwards all requests to the correct
   * DataLogger_ based on the rport of the request.
   */
  class DataLogger_
  {
  public:
    DataLogger_( const DataLoggingRequest&, const RecordablesMap< HostNode >& );
    index
    get_mm_gid() const
    {
      return multimeter_;
    }
    void handle( HostNode&, const DataLoggingRequest& );
    void record_data( const HostNode&, long );
    void reset();
    void init();

  private:
    index multimeter_; //!< GID of multimeter for which the logger works
    size_t num_vars_;  //!< number of variables recorded

    Time recording_interval_; //!< interval between two recordings
    Time recording_offset_;   //!< offset relative to which interval is calculated
    long rec_int_steps_;      //!< interval in steps
    long next_rec_step_;      //!< next time step at which to record

    /** Vector of pointers to member functions for data access. */
    std::vector< typename RecordablesMap< HostNode >::DataAccessFct > node_access_;

    /**
     * Buffer for data.
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
  UniversalDataLogger( const UniversalDataLogger& );

  //! Should not be assigned
  UniversalDataLogger const& operator=( const UniversalDataLogger& );
};

// must be defined in this file, since it is required by check_connection(),
// which typically is in h-files.
template < typename HostNode >
port
nest::UniversalDataLogger< HostNode >::connect_logging_device( const DataLoggingRequest& req,
  const RecordablesMap< HostNode >& rmap )
{
  // rports are assigned consecutively, the caller may not request specific
  // rports.
  if ( req.get_rport() != 0 )
  {
    throw IllegalConnection(
      "UniversalDataLogger::connect_logging_device(): "
      "Connections from multimeter to node must request rport 0." );
  }

  // ensure that we have not connected this multimeter before
  const index mm_gid = req.get_sender().get_gid();
  const size_t n_loggers = data_loggers_.size();
  size_t j = 0;
  while ( j < n_loggers and data_loggers_[ j ].get_mm_gid() != mm_gid )
  {
    ++j;
  }
  if ( j < n_loggers )
  {
    throw IllegalConnection(
      "UniversalDataLogger::connect_logging_device(): "
      "Each multimeter can only be connected once to a given node." );
  }

  // we now know that we have no DataLogger_ for the given multimeter, so we
  // create one and push it
  data_loggers_.push_back( DataLogger_( req, rmap ) );

  // rport is index plus one, i.e., size
  return data_loggers_.size();
}

template < typename HostNode >
nest::UniversalDataLogger< HostNode >::DataLogger_::DataLogger_( const DataLoggingRequest& req,
  const RecordablesMap< HostNode >& rmap )
  : multimeter_( req.get_sender().get_gid() )
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
      throw IllegalConnection(
        "UniversalDataLogger::connect_logging_device(): "
        "Unknown recordable " + recvars[ j ].toString() );
    }

    node_access_.push_back( rec->second );
  }

  num_vars_ = node_access_.size();

  if ( num_vars_ > 0 and req.get_recording_interval() < Time::step( 1 ) )
  {
    throw IllegalConnection(
      "UniversalDataLogger::connect_logging_device(): "
      "recording interval must be >= resolution." );
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
 * nest::aeif_cond_beta_multisynapse::Buffers_::Buffers_(aeif_cond_beta_multisynapse&
 * n)
 * : logger_(n), ... {}
 *
 * nest::aeif_cond_beta_multisynapse::Buffers_::Buffers_(const Buffers_&,
 * aeif_cond_beta_multisynapse& n)
 * : logger_(n), ... {}
 *
 * nest::aeif_cond_beta_multisynapse::aeif_cond_beta_multisynapse()
 * : ..., B_(*this) {}
 *
 * nest::aeif_cond_beta_multisynapse::aeif_cond_beta_multisynapse(const
 * aeif_cond_beta_multisynapse& n)
 * : ..., B_(n.B_, *this) {}
 * @code
 *
 * @todo Could HostNode be passed as const& to handle() and record_data()?
 *
 * @note To avoid inclusion problems and code-bloat, the class
 *       interface is defined in this file, while most of the
 *       implementation is in the companion universal_data_logger_impl.h.
 *       As a consequence, calls to UniversalDataLogger members should
 *       only come from cpp files---do not inline them.
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
  port connect_logging_device( const DataLoggingRequest&, const DynamicRecordablesMap< HostNode >& );

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
   * Has no effect if buffer is initialized already.
   */
  void init();

private:
  /**
   * Single data logger, serving one multimeter.
   * For each multimeter connected to a node, one DataLogger_ instance is
   * created. The UniversalDataLogger forwards all requests to the correct
   * DataLogger_ based on the rport of the request.
   */
  class DataLogger_
  {
  public:
    DataLogger_( const DataLoggingRequest&, const DynamicRecordablesMap< HostNode >& );
    index
    get_mm_gid() const
    {
      return multimeter_;
    }
    void handle( HostNode&, const DataLoggingRequest& );
    void record_data( const HostNode&, long );
    void reset();
    void init();

  private:
    index multimeter_; //!< GID of multimeter for which the logger works
    size_t num_vars_;  //!< number of variables recorded

    Time recording_interval_; //!< interval between two recordings
    Time recording_offset_;   //!< offset relative to which interval is calculated
    long rec_int_steps_;      //!< interval in steps
    long next_rec_step_;      //!< next time step at which to record

    /** Vector of pointers to member functions for data access. */
    std::vector< const typename DynamicRecordablesMap< HostNode >::DataAccessFct* > node_access_;

    /**
     * Buffer for data.
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
port
nest::DynamicUniversalDataLogger< HostNode >::connect_logging_device( const DataLoggingRequest& req,
  const DynamicRecordablesMap< HostNode >& rmap )
{
  // rports are assigned consecutively, the caller may not request specific
  // rports.
  if ( req.get_rport() != 0 )
  {
    throw IllegalConnection(
      "DynamicUniversalDataLogger::connect_logging_device(): "
      "Connections from multimeter to node must request rport 0." );
  }

  // ensure that we have not connected this multimeter before
  const index mm_gid = req.get_sender().get_gid();
  const size_t n_loggers = data_loggers_.size();
  size_t j = 0;
  while ( j < n_loggers && data_loggers_[ j ].get_mm_gid() != mm_gid )
  {
    ++j;
  }
  if ( j < n_loggers )
  {
    throw IllegalConnection(
      "DynamicUniversalDataLogger::connect_logging_device(): "
      "Each multimeter can only be connected once to a given node." );
  }

  // we now know that we have no DataLogger_ for the given multimeter, so we
  // create one and push it
  data_loggers_.push_back( DataLogger_( req, rmap ) );

  // rport is index plus one, i.e., size
  return data_loggers_.size();
}

template < typename HostNode >
nest::DynamicUniversalDataLogger< HostNode >::DataLogger_::DataLogger_( const DataLoggingRequest& req,
  const DynamicRecordablesMap< HostNode >& rmap )
  : multimeter_( req.get_sender().get_gid() )
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
      throw IllegalConnection(
        "DynamicUniversalDataLogger::connect_logging_device(): "
        "Unknown recordable " + recvars[ j ].toString() );
    }

    node_access_.push_back( &( rec->second ) );
  }

  num_vars_ = node_access_.size();

  if ( num_vars_ > 0 && req.get_recording_interval() < Time::step( 1 ) )
  {
    throw IllegalConnection(
      "DynamicUniversalDataLogger::connect_logging_device(): "
      "recording interval must be >= resolution." );
  }

  recording_interval_ = req.get_recording_interval();
  recording_offset_ = req.get_recording_offset();
}
}

#endif // UNIVERSAL_DATA_LOGGER_H
