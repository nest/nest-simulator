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
#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_time.h"
#include "nest_types.h"
#include "spike_data.h"
#include "vp_manager.h"

// Includes from sli:
#include "name.h"

namespace nest
{

class Node;

/**
 * Encapsulate information sent between nodes.
 *
 * Event is the base class for transmitting information between nodes in NEST,
 * with different subclasses for transmitting different types of information. Event
 * types come in three categories
 * -# SpikeEvent can be transmitted between MPI processes
 * -# SecondaryEvent subclasses can also be transmitted between MPI processes, but need to be transmitted via secondary
 connections. They can transport data.
 * -# All other Event subclasses can only be transmitted within an MPI process
 *
 * Events are used for two tasks:
 * -# Creating connections
 * -# Sending signals between nodes during simulation
 *
 * ## Events during connection
 *
 * Node::send_test_event() creates an Event instance of the type of event
 * emitted by that node type, and calls Node::handles_test_event() on the
 * target node. During this call, the event will contain a pointer to a sender
 * node, which is not necessarily the actual sender (which may reside on a
 * different MPI rank), but usually a proxy node. The sender node id is not set.
 * The essential task of this handshake is to ensure that the target can handle
 * the connection and requested receptor type, and to return `rport` information.
 *
 * ## Events during simulation
 *
 * Events transmit information during simulation. SpikeEvent and SecondaryEvent types are first stored
 * in buffers on the sending VP, then serialized for transmission to destination VPs and finally deserialized
 * for delivery. In this process, for the sake of efficiency, NEST creates one Event object and updates its
 * properties for each single event to be delivered. In this case, no pointer to the source node is stored
 * in the Event (as it may be on a different MPI rank), but the correct sender node id is provided.
 *
 * Other Event types are delivered directly on the VP on which they are generated and can, for example, be used
 * for call backs or request-reply sequences.
 *
 * @see Node
 * @see SpikeEvent
 * @see DSSpikeEvent
 * @see RateEvent
 * @see CurrentEvent
 * @see DSCurrentEvent
 * @see ConductanceEvent
 * @see WeightRecorderEvent
 * @see DataLoggingRequest
 * @see DataLoggingReply
 * @see DataEvent
 * @see DoubleDataEvent
 * @see SecondaryEvent
 * @see DelayedRateConnectionEvent
 * @see DiffusionConnectionEvent
 * @see GapJunctionEvent
 * @see InstantaneousRateConnectionEvent

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
   * Return node ID of receiving Node.
   */
  index get_receiver_node_id() const;

  /**
   * Return reference to sending Node.
   *
   * @note This will cause a segmentation fault if sender has not been set via set_sender().
   */
  Node& get_sender() const;

  /**
   * Change pointer to sending Node.
   */
  void set_sender( Node& );

  /**
   * Sender is local. Return node ID of sending Node.
   *
   * @note This will trigger an assertion if sender node id has not been set.
   */
  index get_sender_node_id() const;

  /**
   * Sender is not local. Retrieve node ID of sending Node from SourceTable and return it.
   */
  index retrieve_sender_node_id_from_source_table() const;

  /**
   * Change node ID of sending Node.
   */
  void set_sender_node_id( const index );

  /**
   * Set tid, syn_id, lcid of spike_data_.
   * These are required to retrieve the Node ID of a non-local sender from the SourceTable.
   */
  void set_sender_node_id_info( const thread tid, const synindex syn_id, const index lcid );

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
  virtual void set_drift_factor( weight ) {};

  /**
   * Set diffusion_factor of the event (see DiffusionConnectionEvent).
   */
  virtual void set_diffusion_factor( weight ) {};

  /**
   * Returns true if the pointer to the sender node is valid.
   */
  bool sender_is_valid() const;

  /**
   * Returns true if the pointer to the receiver node is valid.
   */
  bool receiver_is_valid() const;

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
  index sender_node_id_;        //!< node ID of sender or 0
  SpikeData sender_spike_data_; //!< spike data of sender node, in some cases required to retrieve node ID
  /*
   * The original formulation used references to Nodes as
   * members, however, in order to avoid the reference of reference
   * problem, we store sender and receiver as pointers and use
   * references in the interface.
   * Thus, we can still ensure that the pointers are never nullptr.
   */
  Node* sender_;   //!< Pointer to sender or nullptr.
  Node* receiver_; //!< Pointer to receiver or nullptr.


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
  void operator()() override;
  SpikeEvent* clone() const override;

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
  WeightRecorderEvent* clone() const override;
  void operator()() override;

  /**
   * Return node ID of receiving Node.
   */
  index get_receiver_node_id() const;

  /**
   * Change node ID of receiving Node.
   */

  void set_receiver_node_id( index );

protected:
  index receiver_node_id_; //!< node ID of receiver or 0.
};

inline WeightRecorderEvent::WeightRecorderEvent()
  : receiver_node_id_( 0 )
{
}

inline WeightRecorderEvent*
WeightRecorderEvent::clone() const
{
  return new WeightRecorderEvent( *this );
}

inline void
WeightRecorderEvent::set_receiver_node_id( index node_id )
{
  receiver_node_id_ = node_id;
}

inline index
WeightRecorderEvent::get_receiver_node_id() const
{
  return receiver_node_id_;
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
  void operator()() override;
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
  void operator()() override;
  RateEvent* clone() const override;

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
  void operator()() override;
  CurrentEvent* clone() const override;

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
  void operator()() override;
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

  DataLoggingRequest* clone() const override;

  void operator()() override;

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
   * @note This pointer shall be nullptr unless the event is sent by a connection
   * routine.
   */
  std::vector< Name > const* const record_from_;
};

inline DataLoggingRequest::DataLoggingRequest()
  : Event()
  , recording_interval_( Time::neg_inf() )
  , recording_offset_( Time::ms( 0. ) )
  , record_from_( nullptr )
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
  assert( record_from_ );

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

  void operator()() override;

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
  clone() const override
  {
    assert( false );
    return nullptr;
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
  void operator()() override;
  ConductanceEvent* clone() const override;

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
  void operator()() override;
  DoubleDataEvent* clone() const override;
};

inline DoubleDataEvent*
DoubleDataEvent::clone() const
{
  return new DoubleDataEvent( *this );
}

//*************************************************************
// Inline implementations.

inline bool
Event::sender_is_valid() const
{
  return sender_;
}

inline bool
Event::receiver_is_valid() const
{
  return receiver_;
}

inline bool
Event::is_valid() const
{
  return ( sender_is_valid() and receiver_is_valid() and d_ > 0 );
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
Event::set_sender_node_id( const index node_id )
{
  sender_node_id_ = node_id;
}

inline void
Event::set_sender_node_id_info( const thread tid, const synindex syn_id, const index lcid )
{
  // lag and offset of SpikeData are not used here
  sender_spike_data_.set( tid, syn_id, lcid, 0, 0.0 );
}

inline Node&
Event::get_receiver() const
{
  return *receiver_;
}

inline Node&
Event::get_sender() const
{
  return *sender_;
}

inline index
Event::get_sender_node_id() const
{
  assert( sender_node_id_ > 0 );
  return sender_node_id_;
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
