/*
 *  drop_odd_spike_connection.h
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

#ifndef DROP_ODD_SPIKE_CONNECTION_H
#define DROP_ODD_SPIKE_CONNECTION_H

// Includes from nestkernel:
#include "connection.h"


namespace mynest
{

/** @BeginDocumentation
  Name: drop_odd_spike - Synapse dropping spikes with odd time stamps.

  Description:
  This synapse will not deliver any spikes with odd time stamps, while spikes
  with even time stamps go through unchanged.

  Transmits: SpikeEvent

  Remarks:
  This synapse type is provided only for illustration purposes in MyModule.

  SeeAlso: synapsedict
*/
template < typename targetidentifierT >
class DropOddSpikeConnection : public nest::Connection< targetidentifierT >
{
private:
  double weight_; //!< Synaptic weight

public:
  //! Type to use for representing common synapse properties
  typedef nest::CommonSynapseProperties CommonPropertiesType;

  //! Shortcut for base class
  typedef nest::Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  DropOddSpikeConnection()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  //! Default Destructor.
  ~DropOddSpikeConnection()
  {
  }

  /**
   * Helper class defining which types of events can be transmitted.
   *
   * These methods are only used to test whether a certain type of connection
   * can be created.
   *
   * `handles_test_event()` should be added for all event types that the
   * synapse can transmit. The methods shall return `invalid_port_`; the
   * return value will be ignored.
   *
   * Since this is a synapse model dropping spikes, it is only for spikes,
   * therefore we only implement `handles_test_event()` only for spike
   * events.
   *
   * See Kunkel et al (2014), Sec 3.3.1, for background information.
   */
  class ConnTestDummyNode : public nest::ConnTestDummyNodeBase
  {
  public:
    using nest::ConnTestDummyNodeBase::handles_test_event;
    nest::port
    handles_test_event( nest::SpikeEvent&, nest::rport )
    {
      return nest::invalid_port_;
    }

    nest::port
    handles_test_event( nest::DSSpikeEvent&, nest::rport )
    {
      return nest::invalid_port_;
    }
  };

  /**
   * Check that requested connection can be created.
   *
   * This function is a boilerplate function that should be included unchanged
   * in all synapse models. It is called before a connection is added to check
   * that the connection is legal. It is a wrapper that allows us to call
   * the "real" `check_connection_()` method with the `ConnTestDummyNode
   * dummy_target;` class for this connection type. This avoids a virtual
   * function call for better performance.
   *
   * @param s  Source node for connection
   * @param t  Target node for connection
   * @param receptor_type  Receptor type for connection
   */
  void
  check_connection( nest::Node& s,
    nest::Node& t,
    nest::rport receptor_type,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  /**
   * Send an event to the receiver of this connection.
   * @param e The event to send
   * @param t Thread
   * @param cp Common properties to all synapses.
   */
  void send( nest::Event& e, nest::thread t, const CommonPropertiesType& cp );

  // The following methods contain mostly fixed code to forward the
  // corresponding tasks to corresponding methods in the base class and the w_
  // data member holding the weight.

  //! Store connection status information in dictionary
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set connection status.
   *
   * @param d Dictionary with new parameter values
   * @param cm ConnectorModel is passed along to validate new delay values
   */
  void set_status( const DictionaryDatum& d, nest::ConnectorModel& cm );

  //! Allows efficient initialization on contstruction
  void
  set_weight( double w )
  {
    weight_ = w;
  }
};


template < typename targetidentifierT >
inline void
DropOddSpikeConnection< targetidentifierT >::send( nest::Event& e,
  nest::thread t,
  const CommonPropertiesType& props )
{
  if ( e.get_stamp().get_steps() % 2 ) // stamp is odd, drop it
  {
    return;
  }

  // Even time stamp, we send the spike using the normal sending mechanism
  // send the spike to the target
  e.set_weight( weight_ );
  e.set_delay_steps( ConnectionBase::get_delay_steps() );
  e.set_receiver( *ConnectionBase::get_target( t ) );
  e.set_rport( ConnectionBase::get_rport() );
  e(); // this sends the event
}

template < typename targetidentifierT >
void
DropOddSpikeConnection< targetidentifierT >::get_status(
  DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, nest::names::weight, weight_ );
  def< long >( d, nest::names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
DropOddSpikeConnection< targetidentifierT >::set_status(
  const DictionaryDatum& d,
  nest::ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, nest::names::weight, weight_ );
}

} // namespace

#endif // drop_odd_spike_connection.h
