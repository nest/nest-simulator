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

#ifndef GAP_JUNCTION_H
#define GAP_JUNCTION_H

#include "connection.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Synapses
@ingroup gap

Name: gap_junction - Synapse type for gap-junction connections.

Description:

gap_junction is a connector to create gap junctions between pairs
of neurons. Gap junctions are bidirectional connections.
In order to create one accurate gap-junction connection between
neurons i and j two NEST connections are required: For each created
connection a second connection with the exact same parameters in
the opposite direction is required. NEST provides the possibility
to create both connections with a single call to Connect via
the make_symmetric flag:

    i j << /rule /one_to_one /make_symmetric true >> /gap_junction Connect

The value of the parameter "delay" is ignored for connections of
type gap_junction.

Transmits: GapJunctionEvent

References:

\verbatim embed:rst
.. [1] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann,
       M (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal network simulations. Frontiers in
       Neuroinformatics 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022

.. [2] Mancilla JG, Lewis,TJ, Pinto DJ, Rinzel J, Connors BW (2007).
       Synchronization of electrically coupled pairs of inhibitory
       interneurons in neocortex. Journal of Neuroscience 27:2058-2073.
       DOI: https://doi.org/10.1523/JNEUROSCI.2715-06.2007
\endverbatim

Author: Jan Hahne, Moritz Helias, Susanne Kunkel

SeeAlso: synapsedict, hh_psc_alpha_gap
*/
template < typename targetidentifierT >
class GapJunction : public Connection< targetidentifierT >
{

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;
  typedef GapJunctionEvent EventType;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  GapJunction()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  void
  check_connection( Node& s, Node& t, rport receptor_type, const CommonPropertiesType& )
  {
    EventType ge;

    s.sends_secondary_event( ge );
    ge.set_sender( s );
    Connection< targetidentifierT >::target_.set_rport( t.handles_test_event( ge, receptor_type ) );
    Connection< targetidentifierT >::target_.set_target( &t );
  }

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param p The port under which this connection is stored in the Connector.
   */
  void
  send( Event& e, thread t, const CommonSynapseProperties& )
  {
    e.set_weight( weight_ );
    e.set_receiver( *get_target( t ) );
    e.set_rport( get_rport() );
    e();
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_weight( double w )
  {
    weight_ = w;
  }

  void
  set_delay( double )
  {
    throw BadProperty( "gap_junction connection has no delay" );
  }

private:
  double weight_; //!< connection weight
};

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  // We have to include the delay here to prevent
  // errors due to internal calls of
  // this function in SLI/pyNEST
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
GapJunction< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // If the delay is set, we throw a BadProperty
  if ( d->known( names::delay ) )
  {
    throw BadProperty( "gap_junction connection has no delay" );
  }

  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef GAP_JUNCTION_H */
