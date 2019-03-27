/*
 *  tsodyks2_connection.h
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

#ifndef TSODYKS2_CONNECTION_H
#define TSODYKS2_CONNECTION_H

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "connection.h"

namespace nest
{

/** @BeginDocumentation
Name: tsodyks2_synapse - Synapse type with short term plasticity.

Description:

This synapse model implements synaptic short-term depression and short-term
facilitation according to [1] and [2]. It solves Eq (2) from [1] and
modulates U according to eq. (2) of [2].

This connection merely scales the synaptic weight, based on the spike history
and the parameters of the kinetic model. Thus, it is suitable for all types
of synaptic dynamics, that is current or conductance based.

The parameter A_se from the publications is represented by the
synaptic weight. The variable x in the synapse properties is the
factor that scales the synaptic weight.

Parameters:

The following parameters can be set in the status dictionary:
U          double - probability of release increment (U1) [0,1],
                    default=0.5
u          double - Maximum probability of release (U_se) [0,1],
                    default=0.5
x          double - current scaling factor of the weight, default=U
tau_rec    double - time constant for depression in ms, default=800 ms
tau_fac    double - time constant for facilitation in ms, default=0 (off)

Remarks:

Under identical conditions, the tsodyks2_synapse produces
slightly lower peak amplitudes than the tsodyks_synapse. However,
the qualitative behavior is identical. The script
test_tsodyks2_synapse.py in the examples compares the two synapse
models.


References:

[1] Tsodyks, M. V., & Markram, H. (1997). The neural code between neocortical
    pyramidal neurons depends on neurotransmitter release probability.
    PNAS, 94(2), 719-23.
[2] Fuhrmann, G., Segev, I., Markram, H., & Tsodyks, M. V. (2002). Coding of
    temporal information by activity-dependent synapses. Journal of
    neurophysiology, 87(1), 140-8.
[3] Maass, W., & Markram, H. (2002). Synapses as dynamic memory buffers.
    Neural networks, 15(2), 155-61.

Transmits: SpikeEvent

FirstVersion: October 2011

Author: Marc-Oliver Gewaltig, based on tsodyks_synapse by Moritz Helias

SeeAlso: tsodyks_synapse, synapsedict, stdp_synapse, static_synapse
*/
template < typename targetidentifierT >
class Tsodyks2Connection : public Connection< targetidentifierT >
{
public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  Tsodyks2Connection();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  Tsodyks2Connection( const Tsodyks2Connection& );

  /**
   * Default Destructor.
   */
  ~Tsodyks2Connection()
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_delay;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param cp Common properties to all synapses (empty).
   */
  void send( Event& e, thread t, const CommonSynapseProperties& cp );


  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    // Ensure proper overriding of overloaded virtual functions.
    // Return values from functions are ignored.
    using ConnTestDummyNodeBase::handles_test_event;
    port
    handles_test_event( SpikeEvent&, rport )
    {
      return invalid_port_;
    }
  };


  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }


private:
  double weight_;
  double U_;           //!< unit increment of a facilitating synapse
  double u_;           //!< dynamic value of probability of release
  double x_;           //!< current fraction of the synaptic weight
  double tau_rec_;     //!< [ms] time constant for recovery
  double tau_fac_;     //!< [ms] time constant for facilitation
  double t_lastspike_; //!< time point of last spike emitted
};


/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 */
template < typename targetidentifierT >
inline void
Tsodyks2Connection< targetidentifierT >::send( Event& e,
  thread t,
  const CommonSynapseProperties& )
{
  Node* target = get_target( t );
  const double t_spike = e.get_stamp().get_ms();
  const double h = t_spike - t_lastspike_;
  double x_decay = std::exp( -h / tau_rec_ );
  double u_decay = ( tau_fac_ < 1.0e-10 ) ? 0.0 : std::exp( -h / tau_fac_ );

  // now we compute spike number n+1
  x_ = 1. + ( x_ - x_ * u_ - 1. ) * x_decay; // Eq. 5 from reference [3]
  u_ = U_ + u_ * ( 1. - U_ ) * u_decay;      // Eq. 4 from [3]

  // We use the current values for the spike number n.
  e.set_receiver( *target );
  e.set_weight( x_ * u_ * weight_ );
  // send the spike to the target
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  t_lastspike_ = t_spike;
}

template < typename targetidentifierT >
Tsodyks2Connection< targetidentifierT >::Tsodyks2Connection()
  : ConnectionBase()
  , weight_( 1.0 )
  , U_( 0.5 )
  , u_( U_ )
  , x_( 1 )
  , tau_rec_( 800.0 )
  , tau_fac_( 0.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
Tsodyks2Connection< targetidentifierT >::Tsodyks2Connection(
  const Tsodyks2Connection& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , U_( rhs.U_ )
  , u_( rhs.u_ )
  , x_( rhs.x_ )
  , tau_rec_( rhs.tau_rec_ )
  , tau_fac_( rhs.tau_fac_ )
  , t_lastspike_( rhs.t_lastspike_ )
{
}


template < typename targetidentifierT >
void
Tsodyks2Connection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );

  def< double >( d, names::dU, U_ );
  def< double >( d, names::u, u_ );
  def< double >( d, names::tau_rec, tau_rec_ );
  def< double >( d, names::tau_fac, tau_fac_ );
  def< double >( d, names::x, x_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
Tsodyks2Connection< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, names::dU, U_ );
  if ( U_ > 1.0 || U_ < 0.0 )
  {
    throw BadProperty( "U must be in [0,1]." );
  }

  updateValue< double >( d, names::u, u_ );
  if ( u_ > 1.0 || u_ < 0.0 )
  {
    throw BadProperty( "u must be in [0,1]." );
  }

  updateValue< double >( d, names::tau_rec, tau_rec_ );
  if ( tau_rec_ <= 0.0 )
  {
    throw BadProperty( "tau_rec must be > 0." );
  }

  updateValue< double >( d, names::tau_fac, tau_fac_ );
  if ( tau_fac_ < 0.0 )
  {
    throw BadProperty( "tau_fac must be >= 0." );
  }

  updateValue< double >( d, names::x, x_ );
}

} // namespace

#endif // TSODYKS2_CONNECTION_H
