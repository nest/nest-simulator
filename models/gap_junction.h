/*
 *  gap_junction.h
 *
 *  This file is part of NEST
 *
 *  Copyright (C) 2004-2008 by
 *  The NEST Initiative
 *
 *  See the file AUTHORS for details.
 *
 *  Permission is granted to compile and modify
 *  this file for non-commercial use.
 *  See the file LICENSE for details.
 *
 */


/* BeginDocumentation
   Name: gap_junction

   Description:
     static_synapse_hom_wd does not support any kind of plasticity. It simply stores
     the parameters target, and receiver port for each connection and uses a common
     weight and delay for all connections.

   Transmits: GapJEvent

   Parameters:
     No Parameters

   References:
     No References
   FirstVersion: April 2008
   Author: Susanne Kunkel, Moritz Helias
   SeeAlso: synapsedict, static_synapse
*/

#ifndef GAP_JUNCTION_H
#define GAP_JUNCTION_H

#include "hom_w.h"
#include "target_identifier_ptr_rport.h"
#include "target_identifier_index.h"
#include <typeinfo>

namespace nest
{

/**
 * Class representing a static connection. A static connection has the properties weight, delay and
 * receiver port.
 * This class also serves as the base class for dynamic synapses (like TsodyksConnection,
 * STDPConnection).
 * A suitale Connector containing these connections can be obtained from the template
 * GenericConnector.
 */

template < typename targetidentifierT >
class GapJunction : public Connection< targetidentifierT >
{

  HetW w_;

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;
  typedef GapJEvent EventType;


  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */


  /**
   * needed in order for compiler to find functions defined in base class
   */
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;


  void
  check_connection( Node& s, Node& t, rport receptor_type, double_t )
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
    e.set_weight( w_.get_weight() );
    e.set_delay( get_delay_steps() );
    e.set_receiver( *get_target( t ) );
    e.set_rport( get_rport() );
    e();
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void set_status( const DictionaryDatum& d, index p, ConnectorModel& cm );

  void initialize_property_arrays( DictionaryDatum& d ) const;

  void append_properties( DictionaryDatum& d ) const;

  void
  set_weight( double_t w )
  {
    w_.set_weight( w );
  }
};


template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  w_.get_status( d );
  def< long_t >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  w_.set_status( d, cm );
}

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::set_status( const DictionaryDatum& d,
  index p,
  ConnectorModel& cm )
{
  w_.set_status( d, p, cm );
}

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::initialize_property_arrays( DictionaryDatum& d ) const
{
  w_.initialize_property_arrays( d );
}

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::append_properties( DictionaryDatum& d ) const
{
  w_.append_properties( d );
}

} // namespace

#endif /* #ifndef GAP_JUNCTION_H */
