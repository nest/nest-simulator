/*
 *  connection.h
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

#ifndef CONNECTION_H
#define CONNECTION_H

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connection_label.h"
#include "connector_base_impl.h"
#include "delay_checker.h"
#include "event.h"
#include "kernel_manager.h"
#include "nest_names.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node.h"
#include "spikecounter.h"
#include "syn_id_delay.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"

namespace nest
{

class ConnectorModel;

/**
  * Base class for dummy nodes used in connection testing.
  *
  * This class provides a based for dummy node objects that
  * are used to test whether a connection can be established.
  * The base class provides empty implementations of all pure
  * virtual functions of class Node.
  *
  * Each connection class (i.e., each class derived from class
  * template Connection<T>), must derive a concrete ConnTestDummyNode
  * class that overrides method Node::handles_test_event() for all
  * event types that the connection supports.
  *
  * For details, see Kunkel et al, Front Neuroinform 8:78 (2014),
  * Sec 3.3.1. Note that the ConnTestDummyNode class is called
  * "check_helper" in the paper.
  *
  * @ingroup event_interface
  */
class ConnTestDummyNodeBase : public Node
{
  void
  calibrate()
  {
  }
  void
  update( const nest::Time&, long, long )
  {
  }
  void
  set_status( const DictionaryDatum& )
  {
  }
  void
  get_status( DictionaryDatum& ) const
  {
  }
  void
  init_node_( const nest::Node& )
  {
  }
  void
  init_state_( const nest::Node& )
  {
  }
  void
  init_buffers_()
  {
  }
};


/**
 * Base class for representing connections.
 * It provides the mandatory properties receiver port and target,
 * as well as the functions get_status() and set_status()
 * to read and write them. A suitable Connector containing these
 * connections can be obtained from the template GenericConnector.
 *
 * \note Please note that the event received by the send() function is
 * a reference to a single object that is re-used by each Connection.
 * This means that the object must not be changed in the Connection,
 * or if needs to be changed, everything has to be reset after sending
 * (i.e. after Event::operator() has been called).
 */
template < typename targetidentifierT >
class Connection
{

public:
  // this typedef may be overwritten in the derived connection classes in order
  // to attach a specific event type to this connection type, used in secondary
  // connections not used in primary connectors
  typedef SecondaryEvent EventType;

  Connection()
    : target_()
    , syn_id_delay_( 1.0 )
  {
  }

  Connection( const Connection< targetidentifierT >& rhs )
    : target_( rhs.target_ )
    , syn_id_delay_( rhs.syn_id_delay_ )
  {
  }


  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   *
   * @note Target and Rport cannot be changed after a connection has been
   * created.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Check syn_spec dictionary for parameters that are not allowed with the
   * given connection.
   *
   * Will issue warning or throw error if an illegal parameter is found. The
   * method does nothing if no illegal parameter is found.
   *
   * @note Classes requiring checks need to override the function with their own
   * implementation, as this base class implementation does not do anything.
   */
  void check_synapse_params( const DictionaryDatum& d ) const;

  /**
   * Calibrate the delay of this connection to the desired resolution.
   */
  void calibrate( const TimeConverter& );

  /**
   * Return the delay of the connection in ms
   */
  double
  get_delay() const
  {
    return syn_id_delay_.get_delay_ms();
  }

  /**
   * Return the delay of the connection in steps
   */
  long
  get_delay_steps() const
  {
    return syn_id_delay_.delay;
  }

  /**
   * Set the delay of the connection
   */
  void
  set_delay( const double delay )
  {
    syn_id_delay_.set_delay_ms( delay );
  }

  /**
   * Set the delay of the connection in steps
   */
  void
  set_delay_steps( const long delay )
  {
    syn_id_delay_.delay = delay;
  }

  /**
   * Set the synapse id of the connection
   */
  void
  set_syn_id( synindex syn_id )
  {
    syn_id_delay_.syn_id = syn_id;
  }

  /**
   * Get the synapse id of the connection
   */
  synindex
  get_syn_id() const
  {
    return syn_id_delay_.syn_id;
  }

  long
  get_label() const
  {
    return UNLABELED_CONNECTION;
  }

  /**
   * triggers an update of a synaptic weight
   * this function is needed for neuromodulated synaptic plasticity
   */
  void trigger_update_weight( const thread,
    const std::vector< spikecounter >&,
    const double,
    const CommonSynapseProperties& );

  Node*
  get_target( const thread tid ) const
  {
    return target_.get_target_ptr( tid );
  }
  rport
  get_rport() const
  {
    return target_.get_rport();
  }

  /**
   * Sets a flag in the connection to signal that the following connection has
   * the same source.
   *
   * @see source_has_more_targets
   */
  void
  set_source_has_more_targets( const bool more_targets )
  {
    syn_id_delay_.set_source_has_more_targets( more_targets );
  }

  /**
   * Returns a flag denoting whether the connection has source subsequent
   * targets.
   *
   * @see set_source_has_more_targets
   */
  bool
  source_has_more_targets() const
  {
    return syn_id_delay_.source_has_more_targets();
  }

  /**
   * Disables the connection.
   *
   * @see is_disabled
   */
  void
  disable()
  {
    syn_id_delay_.disable();
  }

  /**
   * Returns a flag denoting if the connection is disabled.
   *
   * @see disable
   */
  bool
  is_disabled() const
  {
    return syn_id_delay_.is_disabled();
  }

protected:
  /**
   * This function calls check_connection() on the sender to check if the
   * receiver
   * accepts the event type and receptor type requested by the sender.
   * \param s The source node
   * \param r The target node
   * \param receptor The ID of the requested receptor type
   * \param the last spike produced by the presynaptic neuron (for STDP and
   * maturing connections)
   */
  void check_connection_( Node& dummy_target, Node& source, Node& target, const rport receptor_type );

  /* the order of the members below is critical
     as it influcences the size of the object. Please leave unchanged
     as
     targetidentifierT target_;
     SynIdDelay syn_id_delay_;        //!< syn_id (char) and delay (24 bit) in
     timesteps of this
     connection
  */
  targetidentifierT target_;
  //! syn_id (char) and delay (24 bit) in timesteps of this connection
  SynIdDelay syn_id_delay_;
};


template < typename targetidentifierT >
inline void
Connection< targetidentifierT >::check_connection_( Node& dummy_target,
  Node& source,
  Node& target,
  const rport receptor_type )
{
  // 1. does this connection support the event type sent by source
  // try to send event from source to dummy_target
  // this line might throw an exception
  source.send_test_event( dummy_target, receptor_type, get_syn_id(), true );

  // 2. does the target accept the event type sent by source
  // try to send event from source to target
  // this returns the port of the incoming connection
  // p must be stored in the base class connection
  // this line might throw an exception
  target_.set_rport( source.send_test_event( target, receptor_type, get_syn_id(), false ) );

  // 3. do the events sent by source mean the same thing as they are
  // interpreted in target?
  // note that we here use a bitwise and operation (&), because we interpret
  // each bit in the signal type as a collection of individual flags
  if ( not( source.sends_signal() & target.receives_signal() ) )
  {
    throw IllegalConnection( "Source and target neuron are not compatible (e.g., spiking vs binary neuron)." );
  }

  target_.set_target( &target );
}

template < typename targetidentifierT >
inline void
Connection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  def< double >( d, names::delay, syn_id_delay_.get_delay_ms() );
  target_.get_status( d );
}

template < typename targetidentifierT >
inline void
Connection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& )
{
  double delay;
  if ( updateValue< double >( d, names::delay, delay ) )
  {
    kernel().connection_manager.get_delay_checker().assert_valid_delay_ms( delay );
    syn_id_delay_.set_delay_ms( delay );
  }
  // no call to target_.set_status() because target and rport cannot be changed
}

template < typename targetidentifierT >
inline void
Connection< targetidentifierT >::check_synapse_params( const DictionaryDatum& d ) const
{
}

template < typename targetidentifierT >
inline void
Connection< targetidentifierT >::calibrate( const TimeConverter& tc )
{
  Time t = tc.from_old_steps( syn_id_delay_.delay );
  syn_id_delay_.delay = t.get_steps();

  if ( syn_id_delay_.delay == 0 )
  {
    syn_id_delay_.delay = 1;
  }
}

template < typename targetidentifierT >
inline void
Connection< targetidentifierT >::trigger_update_weight( const thread,
  const std::vector< spikecounter >&,
  const double,
  const CommonSynapseProperties& )
{
  throw IllegalConnection( "Connection does not support updates that are triggered by a volume transmitter." );
}

} // namespace nest

#endif // CONNECTION_H
