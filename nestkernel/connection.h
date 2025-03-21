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
#include "delay_types.h"
#include "event.h"
#include "kernel_manager.h"
#include "nest.h"
#include "nest_names.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node.h"
#include "spikecounter.h"
#include "target_identifier.h"

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
  pre_run_hook() override
  {
  }
  void
  update( const nest::Time&, const long, const long ) override
  {
  }
  void
  set_status( const DictionaryDatum& ) override
  {
  }
  void
  get_status( DictionaryDatum& ) const override
  {
  }
  void
  init_state_() override
  {
  }
  void
  init_buffers_() override
  {
  }
};

/**
 * Base class for representing connections.
 *
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
template < typename targetidentifierT, typename DelayTypeT >
class Connection
{

public:
  // properties used when registering a connection with the ModelManager
  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::NONE;

  Connection()
    : target_()
    , more_targets_( false )
    , disabled_( false )
    , delay_( 1.0 )
  {
    delay_.set_delay_ms( 1.0 );
  }

  Connection( const Connection< targetidentifierT, DelayTypeT >& rhs ) = default;
  Connection& operator=( const Connection< targetidentifierT, DelayTypeT >& rhs ) = default;

  /**
   * Get a pointer to an instance of a SecondaryEvent if this connection supports secondary events.
   *
   * To prevent erronous calls of this function on primary connections, the base class implementation
   * below just contains `assert(false)`.
   */
  SecondaryEvent* get_secondary_event();

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
   *
   * @see ConnectorModel::check_synapse_params
   */
  void check_synapse_params( const DictionaryDatum& d ) const;

  /**
   * Calibrate the delay of this connection to the desired resolution.
   */
  void calibrate( const TimeConverter& );

  /**
   * Framework for STDP with predominantly axonal delays:
   * Correct this synapse and the corresponding previously sent spike
   * taking into account a new post-synaptic spike.
   */
  void correct_synapse_stdp_ax_delay( const size_t tid,
    const double t_last_pre_spike,
    double& weight_revert,
    const double t_post_spike,
    const CommonSynapseProperties& );

  /**
   * Return the proportion of the transmission delay attributed to the dendrite.
   */
  double
  get_dendritic_delay_ms() const
  {
    return delay_.get_dendritic_delay_ms();
  }

  /**
   * Return the proportion of the transmission delay attributed to the dendrite.
   */
  long
  get_dendritic_delay_steps() const
  {
    return delay_.get_dendritic_delay_steps();
  }

  /**
   * Set the proportion of the transmission delay attributed to the dendrite.
   */
  void
  set_dendritic_delay_ms( const double d )
  {
    delay_.set_dendritic_delay_ms( d );
  }

  /**
   * Set the proportion of the transmission delay attributed to the dendrite.
   */
  void
  set_dendritic_delay_steps( const long d )
  {
    delay_.set_dendritic_delay_steps( d );
  }

  /**
   * Set the proportion of the transmission delay attributed to the axon.
   */
  void
  set_axonal_delay_ms( const double d )
  {
    delay_.set_axonal_delay_ms( d );
  }

  /**
   * Get the proportion of the transmission delay attributed to the axon.
   */
  double
  get_axonal_delay_ms() const
  {
    return delay_.get_axonal_delay_ms();
  }

  /**
   * Set the proportion of the transmission delay attributed to the axon.
   */
  void
  set_axonal_delay_steps( const long d )
  {
    delay_.set_axonal_delay_steps( d );
  }

  /**
   * Get the proportion of the transmission delay attributed to the axon.
   */
  double
  get_axonal_delay_steps() const
  {
    return delay_.get_axonal_delay_steps();
  }

  /**
   * Return the delay of the connection in ms
   */
  double
  get_delay_ms() const
  {
    return delay_.get_delay_ms();
  }

  /**
   * Return the delay of the connection in steps
   */
  long
  get_delay_steps() const
  {
    return delay_.get_delay_steps();
  }

  /**
   * Set the delay of the connection
   */
  void
  set_delay_ms( const double d )
  {
    delay_.set_delay_ms( d );
  }

  /**
   * Set the delay of the connection in steps
   */
  void
  set_delay_steps( const long d )
  {
    delay_.set_delay_steps( d );
  }

  long
  get_label() const
  {
    return UNLABELED_CONNECTION;
  }

  /**
   * Triggers an update of a synaptic weight
   *
   * This function is needed for neuromodulated synaptic plasticity
   */
  void trigger_update_weight( const size_t,
    const std::vector< spikecounter >&,
    const double,
    const CommonSynapseProperties& );

  Node*
  get_target( const size_t tid ) const
  {
    return target_.get_target_ptr( tid );
  }
  size_t
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
    more_targets_ = more_targets;
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
    return more_targets_;
  }

  /**
   * Disables the synapse.
   *
   * @see is_disabled
   */
  void
  disable()
  {
    disabled_ = true;
  }

  /**
   * Returns a flag denoting if the synapse is disabled.
   *
   * @see disable
   */
  bool
  is_disabled() const
  {
    return disabled_;
  }

protected:
  /**
   * This function calls check_connection() on the sender to check if the
   * receiver accepts the event type and receptor type requested by the sender.
   * \param s The source node
   * \param r The target node
   * \param receptor The ID of the requested receptor type
   * \param the last spike produced by the presynaptic neuron (for STDP and
   * maturing connections)
   */
  void check_connection_( Node& dummy_target,
    Node& source,
    Node& target,
    const synindex syn_id,
    const size_t receptor_type );

  targetidentifierT target_;
  bool more_targets_ : 1;
  bool disabled_ : 1;
  DelayTypeT delay_;
  // There are still 14 bits to spare here. If more bits are required, the sizes of the delays in the delay struct could
  // be reduced even more as well.
};

using success_connection_target_ptr_size =
  StaticAssert< sizeof( Connection< TargetIdentifierPtrRport, TotalDelay > ) == 24 >::success;
using success_connection_target_idx_size =
  StaticAssert< sizeof( Connection< TargetIdentifierIndex, TotalDelay > ) == 8 >::success;
using success_connection_target_ptr_size =
  StaticAssert< sizeof( Connection< TargetIdentifierPtrRport, AxonalDendriticDelay > ) == 24 >::success;
using success_connection_target_idx_size =
  StaticAssert< sizeof( Connection< TargetIdentifierIndex, AxonalDendriticDelay > ) == 8 >::success;

template < typename targetidentifierT, typename DelayTypeT >
constexpr ConnectionModelProperties Connection< targetidentifierT, DelayTypeT >::properties;

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::check_connection_( Node& dummy_target,
  Node& source,
  Node& target,
  const synindex syn_id,
  const size_t receptor_type )
{
  // 1. does this connection support the event type sent by source try to send event from source to dummy_target this
  // line might throw an exception
  source.send_test_event( dummy_target, receptor_type, syn_id, true );

  // 2. does the target accept the event type sent by source try to send event from source to target
  // this returns the port of the incoming connection p must be stored in the base class connection
  // this line might throw an exception
  target_.set_rport( source.send_test_event( target, receptor_type, syn_id, false ) );

  // 3. do the events sent by source mean the same thing as they are interpreted in target?
  // note that we here use a bitwise and operation (&), because we interpret
  // each bit in the signal type as a collection of individual flags
  if ( not( source.sends_signal() & target.receives_signal() ) )
  {
    throw IllegalConnection( "Source and target neuron are not compatible (e.g., spiking vs binary neuron)." );
  }

  target_.set_target( &target );
}

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::get_status( DictionaryDatum& d ) const
{
  delay_.get_status( d );
  target_.get_status( d );
}

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  delay_.set_status( d, cm );
  // no call to target_.set_status() because target and rport cannot be changed
}

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::check_synapse_params( const DictionaryDatum& ) const
{
}

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::calibrate( const TimeConverter& tc )
{
  delay_.calibrate( tc );
}

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::correct_synapse_stdp_ax_delay( const size_t,
  const double,
  double&,
  const double,
  const CommonSynapseProperties& )
{
  throw IllegalConnection( "Connection does not support correction in case of STDP with predominantly axonal delays." );
}

template < typename targetidentifierT, typename DelayTypeT >
inline void
Connection< targetidentifierT, DelayTypeT >::trigger_update_weight( const size_t,
  const std::vector< spikecounter >&,
  const double,
  const CommonSynapseProperties& )
{
  throw IllegalConnection( "Connection does not support updates that are triggered by a volume transmitter." );
}

template < typename targetidentifierT, typename DelayTypeT >
SecondaryEvent*
Connection< targetidentifierT, DelayTypeT >::get_secondary_event()
{
  assert( false and "Non-primary connections have to provide get_secondary_event()" );
  return nullptr;
}

} // namespace nest

#endif /* CONNECTION_H */
