/*
 *  cont_delay_synapse.h
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

#ifndef CONT_DELAY_SYNAPSE_H
#define CONT_DELAY_SYNAPSE_H


// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "connection.h"
#include "logging_manager.h"

#include "connection_manager.h"
#include "delay_checker.h"
#include "logging.h"

namespace nest
{

/* BeginUserDocs: synapse, continuous delay

Short description
+++++++++++++++++

Synapse type for continuous delays

Description
+++++++++++

``cont_delay_synapse`` relaxes the condition that NEST only implements delays
which are an integer multiple of the time step `h`. A continuous delay is
decomposed into an integer part (``delay_``) and a double (``delay_offset_``) so
that the actual delay is given by  ``delay_*h - delay_offset_``. This can be
combined with off-grid spike times.

All delays set by the normal NEST Connect function will be rounded, even
when using cont_delay_synapse. To set non-grid delays, you must either

1. set the delay as model default using :py:func:`.SetDefaults`, which
   is very efficient, but results in a situation where all synapses then
   will have the same delay.

2. set the delay for each synapse after the connections have been
   created, which is slower, but allows individual delay values.

Continuous delays cannot be shorter than the simulation resolution.

Transmits
+++++++++

SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent, DoubleDataEvent

See also
++++++++

static_synapse, iaf_psc_alpha_ps

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: cont_delay_synapse

EndUserDocs */

void register_cont_delay_synapse( const std::string& name );

template < typename targetidentifierT >
class cont_delay_synapse : public Connection< targetidentifierT >
{

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::SUPPORTS_HPC
    | ConnectionModelProperties::SUPPORTS_LBL | ConnectionModelProperties::SUPPORTS_WFR;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  cont_delay_synapse();

  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  cont_delay_synapse( const cont_delay_synapse& ) = default;

  /**
   * Default Destructor.
   */
  ~cont_delay_synapse()
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;
  using ConnectionBase::set_delay_steps;

  //! Used by ConnectorModel::add_connection() for fast initialization
  void
  set_weight( double w )
  {
    weight_ = w;
  }

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Issue warning if delay is given in syn_spec.
   */
  void check_synapse_params( const DictionaryDatum& d ) const;

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param cp common properties of all synapses (empty).
   */
  bool send( Event& e, size_t t, const CommonSynapseProperties& cp );

  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    // Ensure proper overriding of overloaded virtual functions.
    // Return values from functions are ignored.
    using ConnTestDummyNodeBase::handles_test_event;
    size_t
    handles_test_event( SpikeEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( RateEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DataLoggingRequest&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( CurrentEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( ConductanceEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DoubleDataEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DSSpikeEvent&, size_t ) override
    {
      return invalid_port;
    }
    size_t
    handles_test_event( DSCurrentEvent&, size_t ) override
    {
      return invalid_port;
    }
  };

  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

private:
  double weight_;       //!< synaptic weight
  double delay_offset_; //!< fractional delay < h,
                        //!< total delay = delay_ - delay_offset_
};

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 */
template < typename targetidentifierT >
inline bool
cont_delay_synapse< targetidentifierT >::send( Event& e, size_t t, const CommonSynapseProperties& )
{
  e.set_receiver( *get_target( t ) );
  e.set_weight( weight_ );
  e.set_rport( get_rport() );
  double orig_event_offset = e.get_offset();
  double total_offset = orig_event_offset + delay_offset_;
  // As far as i have seen, offsets are outside of tics regime provided
  // by the Time-class to allow more precise spike-times, hence comparing
  // on the tics level here is not reasonable. Still, the double comparison
  // seems save.
  if ( total_offset < Time::get_resolution().get_ms() )
  {
    e.set_delay_steps( get_delay_steps() );
    e.set_offset( total_offset );
  }
  else
  {
    e.set_delay_steps( get_delay_steps() - 1 );
    e.set_offset( total_offset - Time::get_resolution().get_ms() );
  }
  e();
  // reset offset to original value
  e.set_offset( orig_event_offset );

  return true;
}

template < typename targetidentifierT >
constexpr ConnectionModelProperties cont_delay_synapse< targetidentifierT >::properties;

template < typename targetidentifierT >
cont_delay_synapse< targetidentifierT >::cont_delay_synapse()
  : ConnectionBase()
  , weight_( 1.0 )
  , delay_offset_( 0.0 )
{
}

template < typename targetidentifierT >
void
cont_delay_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );

  def< double >( d, names::weight, weight_ );
  def< double >( d, names::delay, Time( Time::step( get_delay_steps() ) ).get_ms() - delay_offset_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
cont_delay_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );

  updateValue< double >( d, names::weight, weight_ );

  // set delay if mentioned
  double delay;

  if ( updateValue< double >( d, names::delay, delay ) )
  {

    const double h = Time::get_resolution().get_ms();

    double int_delay;
    const double frac_delay = std::modf( delay / h, &int_delay );

    if ( frac_delay == 0 )
    {
      kernel::manager< ConnectionManager >.get_delay_checker().assert_valid_delay_ms( delay );
      set_delay_steps( Time::delay_ms_to_steps( delay ) );
      delay_offset_ = 0.0;
    }
    else
    {
      const long lowerbound = static_cast< long >( int_delay );
      kernel::manager< ConnectionManager >.get_delay_checker().assert_two_valid_delays_steps(
        lowerbound, lowerbound + 1 );
      set_delay_steps( lowerbound + 1 );
      delay_offset_ = h * ( 1.0 - frac_delay );
    }
  }
}

template < typename targetidentifierT >
void
cont_delay_synapse< targetidentifierT >::check_synapse_params( const DictionaryDatum& syn_spec ) const
{
  if ( syn_spec->known( names::delay ) )
  {
    LOG( M_WARNING,
      "Connect",
      "The delay will be rounded to the next multiple of the time step. "
      "To use a more precise time delay it needs to be defined within "
      "the synapse, e.g. with CopyModel()." );
  }
}

} // of namespace nest

#endif // of #ifndef CONT_DELAY_SYNAPSE_H
