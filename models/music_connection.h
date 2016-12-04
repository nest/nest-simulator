/*
 *  music_connection.h
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
  Name: static_synapse - Synapse type for static connections.

  Description:
   static_synapse does not support any kind of plasticity. It simply stores
   the parameters target, weight, delay and receiver port for each connection.

  FirstVersion: October 2005
  Author: Jochen Martin Eppler, Moritz Helias

  Transmits: SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,
  DoubleDataEvent, DataLoggingRequest

  Remarks: Refactored for new connection system design, March 2007

  SeeAlso: synapsedict, tsodyks_synapse, stdp_synapse
*/

#ifndef MUSIC_CONNECTION_H
#define MUSIC_CONNECTION_H

// Includes from nestkernel:
#include "static_connection.h"

namespace nest
{

/**
 * Class representing a static connection. A static connection has the
 * properties weight, delay and receiver port. A suitable Connector containing
 * these connections can be obtained from the template GenericConnector.
 */


template < typename targetidentifierT >
class MusicConnection: public StaticConnection< targetidentifierT >
{
  long music_channel_;

public:
  /* // this line determines which common properties to use */
  typedef CommonSynapseProperties CommonPropertiesType;

  typedef StaticConnection< targetidentifierT > StaticConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  MusicConnection()
    : StaticConnectionBase()
    , music_channel_( 0 )
  {
  }

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  MusicConnection( const MusicConnection& rhs )
    : StaticConnectionBase( rhs )
    , music_channel_( rhs.music_channel_ )
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using StaticConnectionBase::get_delay_steps;
  using StaticConnectionBase::get_rport;
  using StaticConnectionBase::get_target;
  using StaticConnectionBase::set_weight;
  using StaticConnectionBase::check_connection;
  using StaticConnectionBase::send;

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_music_channel( long music_channel )
  {
	music_channel_ = music_channel;
  }
};

template < typename targetidentifierT >
void
MusicConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  StaticConnectionBase::get_status( d );
  def< long >( d, names::music_channel, music_channel_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
MusicConnection< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  StaticConnectionBase::set_status( d, cm );
  updateValue< long >( d, names::music_channel, music_channel_ );
}

} // namespace

#endif /* #ifndef MUSIC_CONNECTION */
