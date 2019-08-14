/*
 *  event.h
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

#ifndef EVENT_H
#define EVENT_H

// C++ includes:
#include <cassert>
#include <cstring>
#include <algorithm>
#include <vector>

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_time.h"
#include "nest_types.h"
#include "vp_manager.h"

// Includes from sli:
#include "name.h"

namespace nest
{

class Node;

/**
 * Encapsulates information which is sent between Nodes.
 *
 * For each type of information there has to be a specialized event
 * class.
 *
 * Events are used for two tasks. During connection, they are used as
 * polymorphic connect objects. During simulation they are used to
 * transport basic event information from one node to the other.
 *
 * A connection between two elements is physically established in two
 * steps: First, create an event with the two envolved elements.
 * Second, call the connect method of the event.
 *
 * An event object contains only administrative information which is
 * needed to successfully deliver the event. Thus, event objects
 * cannot direcly contain custom data: events are not messages. If a
 * node receives an event, arbitrary abounts of data may be exchanged
 * between the participating nodes.

 * With this restriction it is possible to implement a comparatively
 * efficient event handling scheme. 5-6 function calls per event may
 * seem a long time, but this is cheap if we consider that event
 * handling makes update and communication succeptible to parallel
 * execution.
 *
 * @see Node
 * @see SpikeEvent
 * @see RateEvent
 * @see CurrentEvent
 * @see CurrentEvent
 * @see ConductanceEvent
 * @see GapJunctionEvent
 * @see InstantaneousRateConnectionEvent
 * @see DelayedRateConnectionEvent
 * @see DiffusionConnectionEvent
 * @ingroup event_interface
 */

class Event
{

public:
  Event();

  virtual ~Event()
  {
  }

  /**
   * Virtual copy constructor.
   */
  virtual Event* clone() const = 0;

  /**
   * Deliver the event to receiver.
   *
   * This operator calls the handler for the specific event type at
   * the receiver.
   */
  virtual void operator()() = 0;

  /**
   * Change pointer to receiving Node.
   */
  void set_receiver( Node& );

  /**
   * Return reference to receiving Node.
   */
  Node& get_receiver() const;

  /**
   * Return GID of receiving Node.
   */
  index get_receiver_gid() const;

  /**
   * Return reference to sending Node.
   */
  Node& get_sender() const;

  /**
   * Change pointer to sending Node.
   */
  void set_sender( Node& );

  /**
   * Return GID of sending Node.
   */
  index get_sender_gid() const;

  /**
   * Change GID of sending Node.
   */
  void set_sender_gid( index );

  /**
   * Return time stamp of the event.
   * The stamp denotes the time when the event was created.
   * The resolution of Stamp is limited by the time base of the
   * simulation kernel (@see class nest::Time).
   * If this resolution is not fine enough, the creation time
   * can be corrected by using the time attribute.
   */
  Time const& get_stamp() const;

  /**
   * Set the transmission delay of the event.
   * The delay refers to the time until the event is
   * expected to arrive at the receiver.
   * @param t delay.
   */

  void set_delay_steps( delay );

  /**
   * Return transmission delay of the event.
   * The delay refers to the time until the event is
   * expected to arrive at the receiver.
   */
  delay get_delay_steps() const;

  /**
   * Relative spike delivery time in steps.
   * Returns the delivery time of the spike relative to a given
   * time in steps.  Causality commands that the result should
   * not be negative.
   *
   * @returns stamp + delay - 1 - t in steps
   * @param Time reference time
   *
   * @see NEST Time Memo, Rule 3
   */
  long get_rel_delivery_steps( const Time& t ) const;

  /**
   * Return the sender port number of the event.
   * This function returns the number of the port over which the
   * Event was sent.
   * @retval A negative return value indicates that no port number
   * was available.
   */
  port get_port() const;

  /**
   * Return the receiver port number of the event.
   * This function returns the number of the r-port over which the
   * Event was sent.
   * @note A return value of 0 indicates that the r-port is not used.
   */
  rport get_rport() const;

  /**
   * Set the port number.
   * Each event carries the number of the port over which the event
   * is sent. When a connection is established, it receives a unique
   * ID from the sender. This number has to be stored in each Event
   * object.
   * @param p Port number of the connection, or -1 if unknown.
   */
  void set_port( port p );

  /**
   * Set the receiver port number (r-port).
   * When a connection is established, the receiving Node may issue
   * a port number (r-port) to distinguish the incomin
   * connection. By the default, the r-port is not used and its port
   * number defaults to zero.
   * @param p Receiver port number of the connection, or 0 if unused.
   */
  void set_rport( rport p );

  /**
   * Return the creation time offset of the Event.
   * Each Event carries the exact time of creation. This
   * time need not coincide with an integral multiple of the
   * temporal resolution. Rather, Events may be created at any point
   * in time.
   */
  double get_offset() const;

  /**
   * Set the creation time of the Event.
   * Each Event carries the exact time of creation in realtime. This
   * time need not coincide with an integral multiple of the
   * temporal resolution. Rather, Events may be created at any point
   * in time.
   * @param t Creation time in realtime. t has to be in [0, h).
   */
  void set_offset( double t );

  /**
   * Return the weight.
   */
  weight get_weight() const;

  /**
   * Set weight of the event.
   */
  void set_weight( weight t );

  /**
   * Set drift_factor of the event (see DiffusionConnectionEvent).
   */
  virtual void set_drift_factor( weight t ){};

  /**
   * Set diffusion_factor of the event (see DiffusionConnectionEvent).
   */
  virtual void set_diffusion_factor( weight t ){};

  /**
   * Check integrity of the event.
   * This function returns true, if all data, in particular sender
   * and receiver pointers are correctly set.
   */
  bool is_valid() const;

  /**
   * Set the time stamp of the event.
   * The time stamp refers to the time when the event
   * was created.
   */
  void set_stamp( Time const& );

protected:
  index sender_gid_; //!< GID of sender or -1.
                     /*
                      * The original formulation used references to Nodes as
                      * members, however, in order to avoid the reference of reference
                      * problem, we store sender and receiver as pointers and use
                      * references in the interface.
                      * Thus, we can still ensure that the pointers are never NULL.
                      */
  Node* sender_;     //!< Pointer to sender or NULL.
  Node* receiver_;   //!< Pointer to receiver or NULL.


  /**
   * Sender port number.
   * The sender port is used as a unique identifier for the
   * connection.  The receiver of an event can use the port number
   * to obtain data from the sender.  The sender uses this number to
   * locate target-specific information.  @note A negative port
   * number indicates an unknown port.
   */
  port p_;

  /**
   * Receiver port number (r-port).
   * The receiver port (r-port) can be used by the receiving Node to
   * distinguish incoming connections. E.g. the r-port number can be
   * used by Events to access specific parts of a Node. In most
   * cases, however, this port will no be used.
   * @note The use of this port number is optional.
   * @note An r-port number of 0 indicates that the port is not used.
   */
  rport rp_;

  /**
   * Transmission delay.
   * Number of simulations steps that pass before the event is
   * delivered at the receiver.
   * The delay must be at least 1.
   */
  delay d_;

  /**
   * Time stamp.
   * The time stamp specifies the absolute time
   * when the event shall arrive at the target.
   */
  Time stamp_;

  /**
   * Time stamp in steps.
   * Caches the value of stamp in steps for efficiency.
   * Needs to be declared mutable since it is modified
   * by a const function (get_rel_delivery_steps).
   */
  mutable long stamp_steps_;

  /**
   * Offset for precise spike times.
   * offset_ specifies a correction to the creation time.
   * If the resolution of stamp is not sufficiently precise,
   * this attribute can be used to correct the creation time.
   * offset_ has to be in [0, h).
   */
  double offset_;

  /**
   * Weight of the connection.
   */
  weight w_;
};


// Built-in event types
/**
 * Event for spike information.
 * Used to send a spike from one node to the next.
 */
class SpikeEvent : public Event
{
public:
  SpikeEvent();
  void operator()();
  SpikeEvent* clone() const;

  void set_multiplicity( int );
  int get_multiplicity() const;

protected:
  int multiplicity_;
};

inline SpikeEvent::SpikeEvent()
  : multiplicity_( 1 )
{
}

inline SpikeEvent*
SpikeEvent::clone() const
{
  return new SpikeEvent( *this );
}

inline void
SpikeEvent::set_multiplicity( int multiplicity )
{
  multiplicity_ = multiplicity;
}

inline int
SpikeEvent::get_multiplicity() const
{
  return multiplicity_;
}


/**
 * Event for recording the weight of a spike.
 */
class WeightRecorderEvent : public Event
{
public:
  WeightRecorderEvent();
  WeightRecorderEvent* clone() const;
  void operator()();

  /**
   * Return GID of receiving Node.
   */
  index get_receiver_gid() const;

  /**
   * Change GID of receiving Node.
   */

  void set_receiver_gid( index );

protected:
  index receiver_gid_; //!< GID of receiver or 0.
};

inline WeightRecorderEvent::WeightRecorderEvent()
  : receiver_gid_( 0 )
{
}

inline WeightRecorderEvent*
WeightRecorderEvent::clone() const
{
  return new WeightRecorderEvent( *this );
}

inline void
WeightRecorderEvent::set_receiver_gid( index gid )
{
  receiver_gid_ = gid;
}

inline index
WeightRecorderEvent::get_receiver_gid( void ) const
{
  return receiver_gid_;
}


/**
 * "Callback request event" for use in Device.
 *
 * Some Nodes want to perform a function on an event for each
 * of their targets. An example is the poisson_generator which
 * needs to draw a random number for each target. The DSSpikeEvent,
 * DirectSendingSpikeEvent, calls sender->event_hook(*this)
 * in its operator() function instead of calling receiver->handle().
 * The default implementation of Node::event_hook() just calls
 * target->handle(DSSpikeEvent&). Any reimplementation must also
 * execute this call. Otherwise the event will not be delivered.
 * If needed, target->handle(DSSpikeEvent&) may be called more than
 * once.
 *
 * @note Callback events must only be sent via static_synapse
 */
class DSSpikeEvent : public SpikeEvent
{
public:
  void operator()();
};

/**
 * Event for firing rate information.
 * Used to send firing rate from one node to the next.
 * The rate information is not contained in the event
 * object. Rather, the receiver has to poll this information
 * from the sender.
 */
class RateEvent : public Event
{
  double r_;

public:
  void operator()();
  RateEvent* clone() const;

  void set_rate( double );
  double get_rate() const;
};

inline RateEvent*
RateEvent::clone() const
{
  return new RateEvent( *this );
}

inline void
RateEvent::set_rate( double r )
{
  r_ = r;
}

inline double
RateEvent::get_rate() const
{
  return r_;
}

/**
 * Event for electrical currents.
 * Used to send currents from one node to the next.
 */
class CurrentEvent : public Event
{
  double c_;

public:
  void operator()();
  CurrentEvent* clone() const;

  void set_current( double );
  double get_current() const;
};

inline CurrentEvent*
CurrentEvent::clone() const
{
  return new CurrentEvent( *this );
}

inline void
CurrentEvent::set_current( double c )
{
  c_ = c;
}

inline double
CurrentEvent::get_current() const
{
  return c_;
}

/**
 * "Callback request event" for use in Device.
 *
 * Some Nodes want to perform a function on an event for each
 * of their targets. An example is the noise_generator which
 * needs to draw a random number for each target. The DSCurrentEvent,
 * DirectSendingCurrentEvent, calls sender->event_hook(*this)
 * in its operator() function instead of calling receiver->handle().
 * The default implementation of Node::event_hook() just calls
 * target->handle(DSCurrentEvent&). Any reimplementation must also
 * execute this call. Otherwise the event will not be delivered.
 * If needed, target->handle(DSCurrentEvent&) may be called more than
 * once.
 *
 * @note Callback events must only be sent via static_synapse.
 */
class DSCurrentEvent : public CurrentEvent
{
public:
  void operator()();
};

/**
 * @defgroup DataLoggingEvents Event types for analog logging devices.
 *
 * Events for flexible data recording.
 *
 * @addtogroup Devices
 * @ingroup eventinterfaces
 */

/**
 * Request data to be logged/logged data to be sent.
 *
 * @see DataLoggingReply
 * @ingroup DataLoggingEvents
 */
class DataLoggingRequest : public Event
{
public:
  /** Create empty request for use during simulation. */
  DataLoggingRequest();

  DataLoggingRequest( const Time&, const std::vector< Name >& );

  /** Create event for given time interval, offset for interval start,
   *  and vector of recordables. */
  DataLoggingRequest( const Time&, const Time&, const std::vector< Name >& );

  DataLoggingRequest* clone() const;

  void operator()();

  /** Access to stored time interval.*/
  const Time& get_recording_interval() const;

  /** Access to stored offset.*/
  const Time& get_recording_offset() const;

  /** Access to vector of recordables. */
  const std::vector< Name >& record_from() const;

private:
  //! Interval between two recordings, first is step 1
  Time recording_interval_;

  //! Offset relative to which the intervals are computed
  Time recording_offset_;
  /**
   * Names of properties to record from.
   * @note This pointer shall be NULL unless the event is sent by a connection
   * routine.
   */
  std::vector< Name > const* const record_from_;
};

inline DataLoggingRequest::DataLoggingRequest()
  : Event()
  , recording_interval_( Time::neg_inf() )
  , recording_offset_( Time::ms( 0. ) )
  , record_from_( 0 )
{
}

inline DataLoggingRequest::DataLoggingRequest( const Time& rec_int, const std::vector< Name >& recs )
  : Event()
  , recording_interval_( rec_int )
  , record_from_( &recs )
{
}

inline DataLoggingRequest::DataLoggingRequest( const Time& rec_int,
  const Time& rec_offset,
  const std::vector< Name >& recs )
  : Event()
  , recording_interval_( rec_int )
  , recording_offset_( rec_offset )
  , record_from_( &recs )
{
}


inline DataLoggingRequest*
DataLoggingRequest::clone() const
{
  return new DataLoggingRequest( *this );
}

inline const Time&
DataLoggingRequest::get_recording_interval() const
{
  // During simulation, events are created without recording interval
  // information. On these, get_recording_interval() must not be called.
  assert( recording_interval_.is_finite() );

  return recording_interval_;
}

inline const Time&
DataLoggingRequest::get_recording_offset() const
{
  assert( recording_offset_.is_finite() );
  return recording_offset_;
}

inline const std::vector< Name >&
DataLoggingRequest::record_from() const
{
  // During simulation, events are created without recordables
  // information. On these, record_from() must not be called.
  assert( record_from_ != 0 );

  return *record_from_;
}

/**
 * Provide logged data through request transmitting reference.
 * @see DataLoggingRequest
 * @ingroup DataLoggingEvents
 */
class DataLoggingReply : public Event
{
public:
  //! Data type data at single recording time
  typedef std::vector< double > DataItem;

  /** Data item with pertaining time stamp.
   * Items are initialized with time stamp -inf to mark them as invalid.
   * Data is initialized to <double>::max() as a highly implausible value.
   * Ideally, we should initialized to a NaN, but since the C++-standard does
   * not require NaN, that would result in unportable code. max() should draw
   * the users att
   */
  struct Item
  {
    Item( size_t n )
      : data( n, std::numeric_limits< double >::max() )
      , timestamp( Time::neg_inf() )
    {
    }
    DataItem data;
    Time timestamp;
  };

  //! Container for entries
  typedef std::vector< Item > Container;

  //! Construct with reference to data and time stamps to transmit
  DataLoggingReply( const Container& );

  void operator()();

  //! Access referenced data
  const Container&
  get_info() const
  {
    return info_;
  }

private:
  //! Prohibit copying
  DataLoggingReply( const DataLoggingReply& );

  //! Prohibit cloning
  DataLoggingReply*
  clone() const
  {
    assert( false );
    return 0;
  }

  //! data to be transmitted, with time stamps
  const Container& info_;
};

inline DataLoggingReply::DataLoggingReply( const Container& d )
  : Event()
  , info_( d )
{
}

/**
 * Event for electrical conductances.
 * Used to send conductance from one node to the next.
 * The conductance is contained in the event object.
 */
class ConductanceEvent : public Event
{
  double g_;

public:
  void operator()();
  ConductanceEvent* clone() const;

  void set_conductance( double );
  double get_conductance() const;
};

inline ConductanceEvent*
ConductanceEvent::clone() const
{
  return new ConductanceEvent( *this );
}

inline void
ConductanceEvent::set_conductance( double g )
{
  g_ = g;
}

inline double
ConductanceEvent::get_conductance() const
{
  return g_;
}


/**
 * Event for transmitting arbitrary data.
 * This event type may be used for transmitting arbitrary
 * data between events, e.g., images or their FFTs.
 * A shared_ptr to the data is transmitted.  The date type
 * is given as a template parameter.
 * @note: Data is passed via a shared_ptr.
 *        The receiver should copy the data at once, otherwise
 *        it may be modified by the sender.
 *        I hope this scheme is thread-safe, as long as the
 *        receiver copies the data at once.  HEP.
 * @note: This is a base class.  Actual event types had to
 *        be derived, since operator() cannot be instantiated
 *        otherwise.
 */
template < typename D >
class DataEvent : public Event
{
  std::shared_ptr< D > data_;

public:
  void set_pointer( D& data );
  std::shared_ptr< D > get_pointer() const;
};

template < typename D >
inline void
DataEvent< D >::set_pointer( D& data )
{
  data_ = data;
}

template < typename D >
inline std::shared_ptr< D >
DataEvent< D >::get_pointer() const
{
  return data_;
}

class DoubleDataEvent : public DataEvent< double >
{
public:
  void operator()();
  DoubleDataEvent* clone() const;
};

inline DoubleDataEvent*
DoubleDataEvent::clone() const
{
  return new DoubleDataEvent( *this );
}

/**
 * Base class of secondary events. Provides interface for
 * serialization and deserialization. This event type may be
 * used to transmit data on a regular basis
 * Further information about secondary events and
 * their usage with gap junctions can be found in
 *
 * Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,
 * Bolten, M., Frommer, A. and Diesmann, M.,
 * A unified framework for spiking and gap-junction interactions
 * in distributed neuronal network simulations,
 * Front. Neuroinform. 9:22. (2015),
 * doi: 10.3389/fninf.2015.00022
 */
class SecondaryEvent : public Event
{

public:
  virtual SecondaryEvent* clone() const = 0;

  virtual void add_syn_id( const synindex synid ) = 0;

  virtual bool supports_syn_id( const synindex synid ) const = 0;

  //! size of event in units of unsigned int
  virtual size_t size() = 0;
  virtual std::vector< unsigned int >::iterator& operator<<( std::vector< unsigned int >::iterator& pos ) = 0;
  virtual std::vector< unsigned int >::iterator& operator>>( std::vector< unsigned int >::iterator& pos ) = 0;

  virtual const std::vector< synindex >& get_supported_syn_ids() const = 0;

  virtual void reset_supported_syn_ids() = 0;
};

/**
 * This template function returns the number of uints covered by a variable of
 * type T. This function is used to determine the storage demands for a
 * variable of type T in the NEST communication buffer, which is of type
 * std::vector<unsigned int>.
 */
template < typename T >
size_t
number_of_uints_covered( void )
{
  size_t num_uints = sizeof( T ) / sizeof( unsigned int );
  if ( num_uints * sizeof( unsigned int ) < sizeof( T ) )
  {
    num_uints += 1;
  }
  return num_uints;
}

/**
 * This template function writes data of type T to a given position of a
 * std::vector< unsigned int >.
 * Please note that this function does not increase the size of the vector,
 * it just writes the data to the position given by the iterator.
 * The function is used to write data from SecondaryEvents to the NEST
 * communication buffer. The pos iterator is advanced during execution.
 * For a discussion on the functionality of this function see github issue #181
 * and pull request #184.
 */
template < typename T >
void
write_to_comm_buffer( T d, std::vector< unsigned int >::iterator& pos )
{
  // there is no aliasing problem here, since cast to char* invalidate strict
  // aliasing assumptions
  char* const c = reinterpret_cast< char* >( &d );

  const size_t num_uints = number_of_uints_covered< T >();
  size_t left_to_copy = sizeof( T );

  for ( size_t i = 0; i < num_uints; i++ )
  {
    memcpy( &( *( pos + i ) ), c + i * sizeof( unsigned int ), std::min( left_to_copy, sizeof( unsigned int ) ) );
    left_to_copy -= sizeof( unsigned int );
  }

  pos += num_uints;
}

/**
 * This template function reads data of type T from a given position of a
 * std::vector< unsigned int >. The function is used to read SecondaryEvents
 * data from the NEST communication buffer. The pos iterator is advanced
 * during execution. For a discussion on the functionality of this function see
 * github issue #181 and pull request #184.
 */
template < typename T >
void
read_from_comm_buffer( T& d, std::vector< unsigned int >::iterator& pos )
{
  // there is no aliasing problem here, since cast to char* invalidate strict
  // aliasing assumptions
  char* const c = reinterpret_cast< char* >( &d );

  const size_t num_uints = number_of_uints_covered< T >();
  size_t left_to_copy = sizeof( T );

  for ( size_t i = 0; i < num_uints; i++ )
  {
    memcpy( c + i * sizeof( unsigned int ), &( *( pos + i ) ), std::min( left_to_copy, sizeof( unsigned int ) ) );
    left_to_copy -= sizeof( unsigned int );
  }

  pos += num_uints;
}

/**
 * Template class for the storage and communication of a std::vector of type
 * DataType. The class provides the functionality to communicate homogeneous
 * data of type DataType. The second template type Subclass (which should be
 * chosen as the derived class itself) is used to distinguish derived classes
 * with the same DataType. This is required because of the included static
 * variables in the base class (as otherwise all derived classes with the same
 * DataType would share the same static variables).
 *
 * Technically the DataSecondaryEvent only contains iterators pointing to
 * the memory location of the std::vector< DataType >.
 *
 * Conceptually, there is a one-to-one mapping between a SecondaryEvent
 * and a SecondaryConnectorModel. The synindex of this particular
 * SecondaryConnectorModel is stored as first element in the static vector
 * supported_syn_ids_ on model registration. There are however reasons (e.g.
 * the usage of CopyModel or the creation of the labeled synapse model
 * duplicates for pyNN) which make it necessary to register several
 * SecondaryConnectorModels with one SecondaryEvent. Therefore the synindices
 * of all these models are added to supported_syn_ids_. The
 * supports_syn_id()-function allows testing if a particular synid is mapped
 * with the SecondaryEvent in question.
 */
template < typename DataType, typename Subclass >
class DataSecondaryEvent : public SecondaryEvent
{
private:
  // we chose std::vector over std::set because we expect this to be short
  static std::vector< synindex > pristine_supported_syn_ids_;
  static std::vector< synindex > supported_syn_ids_;
  static size_t coeff_length_; // length of coeffarray

  union CoeffarrayBegin
  {
    std::vector< unsigned int >::iterator as_uint;
    typename std::vector< DataType >::iterator as_d;

    CoeffarrayBegin(){}; // need to provide default constructor due to
                         // non-trivial constructors of iterators
  } coeffarray_begin_;

  union CoeffarrayEnd
  {
    std::vector< unsigned int >::iterator as_uint;
    typename std::vector< DataType >::iterator as_d;

    CoeffarrayEnd(){}; // need to provide default constructor due to
                       // non-trivial constructors of iterators
  } coeffarray_end_;

public:
  /**
   * This function is needed to set the synid on model registration.
   * At this point no object of this type is available and the
   * add_syn_id-function cannot be used as it is virtual in the base class
   * and therefore cannot be declared as static.
   */
  static void
  set_syn_id( const synindex synid )
  {
    VPManager::assert_single_threaded();
    pristine_supported_syn_ids_.push_back( synid );
    supported_syn_ids_.push_back( synid );
  }

  /**
   * This function is needed to add additional synids when the
   * corresponded connector model is copied.
   * This function needs to be a virtual function of the base class as
   * it is called from a pointer on SecondaryEvent.
   */
  void
  add_syn_id( const synindex synid )
  {
    assert( not supports_syn_id( synid ) );
    VPManager::assert_single_threaded();
    supported_syn_ids_.push_back( synid );
  }

  const std::vector< synindex >&
  get_supported_syn_ids() const
  {
    return supported_syn_ids_;
  }

  /**
   * Resets the vector of supported syn ids to those originally
   * registered via ModelsModule or user defined Modules, i.e.,
   * removes all syn ids created by CopyModel. This is important to
   * maintain consistency across ResetKernel, which removes all copied
   * models.
   */
  void
  reset_supported_syn_ids()
  {
    supported_syn_ids_.clear();
    for ( size_t i = 0; i < pristine_supported_syn_ids_.size(); ++i )
    {
      supported_syn_ids_.push_back( pristine_supported_syn_ids_[ i ] );
    }
  }

  static void
  set_coeff_length( const size_t coeff_length )
  {
    VPManager::assert_single_threaded();
    coeff_length_ = coeff_length;
  }

  bool
  supports_syn_id( const synindex synid ) const
  {
    return ( std::find( supported_syn_ids_.begin(), supported_syn_ids_.end(), synid ) != supported_syn_ids_.end() );
  }

  void
  set_coeffarray( std::vector< DataType >& ca )
  {
    coeffarray_begin_.as_d = ca.begin();
    coeffarray_end_.as_d = ca.end();
    assert( coeff_length_ == ca.size() );
  }

  /**
   * The following operator is used to read the information of the
   * DataSecondaryEvent from the buffer in EventDeliveryManager::deliver_events
   */
  std::vector< unsigned int >::iterator& operator<<( std::vector< unsigned int >::iterator& pos )
  {
    // The synid can be skipped here as it is stored in a static vector

    // generating a copy of the coeffarray is too time consuming
    // therefore we save an iterator to the beginning+end of the coeffarray
    coeffarray_begin_.as_uint = pos;

    pos += coeff_length_ * number_of_uints_covered< DataType >();

    coeffarray_end_.as_uint = pos;

    return pos;
  }

  /**
   * The following operator is used to write the information of the
   * DataSecondaryEvent into the secondary_events_buffer_.
   * All DataSecondaryEvents are identified by the synid of the
   * first element in supported_syn_ids_.
   */
  std::vector< unsigned int >::iterator& operator>>( std::vector< unsigned int >::iterator& pos )
  {
    for ( typename std::vector< DataType >::iterator it = coeffarray_begin_.as_d; it != coeffarray_end_.as_d; ++it )
    {
      // we need the static_cast here as the size of a stand-alone variable
      // and a std::vector entry may differ (e.g. for std::vector< bool >)
      write_to_comm_buffer( static_cast< DataType >( *it ), pos );
    }
    return pos;
  }

  size_t
  size()
  {
    size_t s = number_of_uints_covered< synindex >();
    s += number_of_uints_covered< index >();
    s += number_of_uints_covered< DataType >() * coeff_length_;

    return s;
  }

  const std::vector< unsigned int >::iterator&
  begin()
  {
    return coeffarray_begin_.as_uint;
  }

  const std::vector< unsigned int >::iterator&
  end()
  {
    return coeffarray_end_.as_uint;
  }

  DataType get_coeffvalue( std::vector< unsigned int >::iterator& pos );
};

/**
 * Event for gap-junction information. The event transmits the interpolation
 * of the membrane potential to the connected neurons.
 */
class GapJunctionEvent : public DataSecondaryEvent< double, GapJunctionEvent >
{

public:
  GapJunctionEvent()
  {
  }

  void operator()();
  GapJunctionEvent* clone() const;
};

/**
 * Event for rate model connections without delay. The event transmits
 * the rate to the connected neurons.
 */
class InstantaneousRateConnectionEvent : public DataSecondaryEvent< double, InstantaneousRateConnectionEvent >
{

public:
  InstantaneousRateConnectionEvent()
  {
  }

  void operator()();
  InstantaneousRateConnectionEvent* clone() const;
};

/**
 * Event for rate model connections with delay. The event transmits
 * the rate to the connected neurons.
 */
class DelayedRateConnectionEvent : public DataSecondaryEvent< double, DelayedRateConnectionEvent >
{

public:
  DelayedRateConnectionEvent()
  {
  }

  void operator()();
  DelayedRateConnectionEvent* clone() const;
};

/**
 * Event for diffusion connections (rate model connections for the
 * siegert_neuron). The event transmits the rate to the connected neurons.
 */
class DiffusionConnectionEvent : public DataSecondaryEvent< double, DiffusionConnectionEvent >
{
private:
  // drift factor of the corresponding connection
  weight drift_factor_;
  // diffusion factor of the corresponding connection
  weight diffusion_factor_;

public:
  DiffusionConnectionEvent()
  {
  }

  void operator()();
  DiffusionConnectionEvent* clone() const;

  void
  set_diffusion_factor( weight t )
  {
    diffusion_factor_ = t;
  };

  void
  set_drift_factor( weight t )
  {
    drift_factor_ = t;
  };

  weight get_drift_factor() const;
  weight get_diffusion_factor() const;
};

template < typename DataType, typename Subclass >
inline DataType
DataSecondaryEvent< DataType, Subclass >::get_coeffvalue( std::vector< unsigned int >::iterator& pos )
{
  DataType elem;
  read_from_comm_buffer( elem, pos );
  return elem;
}

template < typename Datatype, typename Subclass >
std::vector< synindex > DataSecondaryEvent< Datatype, Subclass >::pristine_supported_syn_ids_;

template < typename DataType, typename Subclass >
std::vector< synindex > DataSecondaryEvent< DataType, Subclass >::supported_syn_ids_;

template < typename DataType, typename Subclass >
size_t DataSecondaryEvent< DataType, Subclass >::coeff_length_ = 0;

inline GapJunctionEvent*
GapJunctionEvent::clone() const
{
  return new GapJunctionEvent( *this );
}

inline InstantaneousRateConnectionEvent*
InstantaneousRateConnectionEvent::clone() const
{
  return new InstantaneousRateConnectionEvent( *this );
}

inline DelayedRateConnectionEvent*
DelayedRateConnectionEvent::clone() const
{
  return new DelayedRateConnectionEvent( *this );
}

inline DiffusionConnectionEvent*
DiffusionConnectionEvent::clone() const
{
  return new DiffusionConnectionEvent( *this );
}

inline weight
DiffusionConnectionEvent::get_drift_factor() const
{
  return drift_factor_;
}

inline weight
DiffusionConnectionEvent::get_diffusion_factor() const
{
  return diffusion_factor_;
}

//*************************************************************
// Inline implementations.

inline bool
Event::is_valid() const
{
  return ( ( sender_ != NULL ) and ( receiver_ != NULL ) and ( d_ > 0 ) );
}

inline void
Event::set_receiver( Node& r )
{
  receiver_ = &r;
}

inline void
Event::set_sender( Node& s )
{
  sender_ = &s;
}

inline void
Event::set_sender_gid( index gid )
{
  sender_gid_ = gid;
}

inline Node&
Event::get_receiver( void ) const
{
  return *receiver_;
}

inline Node&
Event::get_sender( void ) const
{
  return *sender_;
}

inline index
Event::get_sender_gid( void ) const
{
  assert( sender_gid_ > 0 );
  return sender_gid_;
}

inline weight
Event::get_weight() const
{
  return w_;
}

inline void
Event::set_weight( weight w )
{
  w_ = w;
}

inline Time const&
Event::get_stamp() const
{
  return stamp_;
}

inline void
Event::set_stamp( Time const& s )
{
  stamp_ = s;
  stamp_steps_ = 0; // setting stamp_steps to zero indicates
                    // stamp_steps needs to be recalculated from
                    // stamp_ next time it is needed (e.g., in
                    // get_rel_delivery_steps)
}

inline delay
Event::get_delay_steps() const
{
  return d_;
}

inline long
Event::get_rel_delivery_steps( const Time& t ) const
{
  if ( stamp_steps_ == 0 )
  {
    stamp_steps_ = stamp_.get_steps();
  }
  return stamp_steps_ + d_ - 1 - t.get_steps();
}

inline void
Event::set_delay_steps( delay d )
{
  d_ = d;
}

inline double
Event::get_offset() const
{
  return offset_;
}

inline void
Event::set_offset( double t )
{
  offset_ = t;
}

inline port
Event::get_port() const
{
  return p_;
}

inline rport
Event::get_rport() const
{
  return rp_;
}

inline void
Event::set_port( port p )
{
  p_ = p;
}

inline void
Event::set_rport( rport rp )
{
  rp_ = rp;
}
}

#endif // EVENT_H
