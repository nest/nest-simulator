/*
 *  gap_junction.h
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


/* BeginDocumentation
   Name: gap_junction

   Description:
     TODO

   Transmits: GapJEvent

   Parameters:
     No Parameters

   References:
     TODO
   FirstVersion: April 2008
   Author: Susanne Kunkel, Moritz Helias
   SeeAlso: synapsedict, static_synapse
*/

#ifndef GAP_JUNCTION_H
#define GAP_JUNCTION_H

#include "connection.h"

namespace nest
{

/**
 * Class representing a gap-junction connection. A gap-junction connection has the properties weight, delay and
 * receiver port.
 * TODO
 * 
 * 
 * 
 */

template < typename targetidentifierT >
class GapJunction : public Connection< targetidentifierT >
{

  double_t weight_;

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;
  typedef GapJEvent EventType;


  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  GapJunction()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }


  // Explicitly declare all methods inherited from the dependent base ConnectionBase.
  // This avoids explicit name prefixes in all places these functions are used.
  // Since ConnectionBase depends on the template parameter, they are not automatically
  // found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;


  void
  check_connection( Node& s, Node& t, rport receptor_type, double_t, const CommonPropertiesType&  )
  {
    // std::cout << "gap_junction::check_connection" << std::endl;
    EventType ge;

    // check, if the sender wants to send events of type EventType
    // this will throw IllegalConnection, if s does not send this event type

    // std::cout << "type of pre node " << typeid(s).name() << std::endl;
    // std::cout << "pre check" << std::endl;

    // this check fails, if the source is on a remote machine,
    // because it is then represented by a proxynode, which
    // does not know about the secondary events
    // could we circumvent this by using a node of the correct type
    // as a proxy for the source instead of a proxynode?
    s.sends_secondary_event( ge );

    // check, if the target can handle events of type
    // EventType
    // std::cout << "post check" << std::endl;
    ge.set_sender( s );
    Connection< targetidentifierT >::target_.set_rport( t.handles_test_event( ge, receptor_type ) );
    Connection< targetidentifierT >::target_.set_target( &t );
  }

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param p The port under which this connection is stored in the Connector.
   * \param t_lastspike Time point of last spike emitted
   */
  void
  send( Event& e, thread t, double_t, const CommonSynapseProperties& )
  {
    e.set_weight( weight_ );
    e.set_delay( get_delay_steps() );
    e.set_receiver( *get_target( t ) );
    e.set_rport( get_rport() );
    e();
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_weight( double_t w )
  {
    weight_ = w;
  }
};

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  ConnectionBase::get_status( d );
  def< double_t >( d, names::weight, weight_ );
  def< long_t >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double_t >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef GAP_JUNCTION_H */
