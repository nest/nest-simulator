/*
 *  tsodyks_connection_hom.h
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

#ifndef TSODYKS_CONNECTION_HOM_H
#define TSODYKS_CONNECTION_HOM_H


// Includes from nestkernel:
#include "common_properties_hom_w.h"
#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, short-term plasticity

Short description
+++++++++++++++++

Synapse type with short term plasticity using homogeneous parameters

Description
+++++++++++

This synapse model implements synaptic short-term depression and short-term
facilitation according to [1]_. In particular it solves Eqs (3) and (4) from
this paper in an exact manner.

Synaptic depression is motivated by depletion of vesicles in the readily
releasable pool of synaptic vesicles (variable x in equation (3)). Synaptic
facilitation comes about by a presynaptic increase of release probability,
which is modeled by variable U in Eq (4).
The original interpretation of variable y is the amount of glutamate
concentration in the synaptic cleft. In [1]_ this variable is taken to be
directly proportional to the synaptic current caused in the postsynaptic
neuron (with the synaptic weight w as a proportionality constant). In order
to reproduce the results of [1]_ and to use this model of synaptic plasticity
in its original sense, the user therefore has to ensure the following
conditions:

1.) The postsynaptic neuron must be of type iaf_psc_exp or iaf_psc_exp_htum,
because these neuron models have a postsynaptic current which decays
exponentially.

2.) The time constant of each tsodyks_synapse targeting a particular neuron
must be chosen equal to that neuron's synaptic time constant. In particular
that means that all synapses targeting a particular neuron have the same
parameter tau_psc.

However, there are no technical restrictions using this model of synaptic
plasticity also in conjunction with neuron models that have a different
dynamics for their synaptic current or conductance. The effective synaptic
weight, which will be transmitted to the postsynaptic neuron upon occurrence
of a spike at time t is u(t)*x(t)*w, where u(t) and x(t) are defined in
Eq (3) and (4), w is the synaptic weight specified upon connection.
The interpretation is as follows: The quantity u(t)*x(t) is the release
probability times the amount of releasable synaptic vesicles at time t of the
presynaptic neuron's spike, so this equals the amount of transmitter expelled
into the synaptic cleft.
The amount of transmitter than relaxes back to 0 with time constant tau_psc
of the synapse's variable y. Since the dynamics of y(t) is linear, the
postsynaptic neuron can reconstruct from the amplitude of the synaptic
impulse u(t)*x(t)*w the full shape of y(t). The postsynaptic neuron, however,
might choose to have a synaptic current that is not necessarily identical to
the concentration of transmitter y(t) in the synaptic cleft. It may realize
an arbitrary postsynaptic effect depending on y(t).

Parameters
++++++++++

========  ======  ========================================================
 U        real    Parameter determining the increase in u with each spike
                  [0,1]
 tau_psc  ms      Time constant of synaptic current
 tau_fac  ms      Time constant for facilitation
 tau_rec  ms      Time constant for depression
 x        real    Initial fraction of synaptic vesicles in the readily
                  releasable pool [0,1]
 y        real    Initial fraction of synaptic vesicles in the synaptic
                  cleft [0,1]
========  ======  ========================================================

Remarks:

The weight and the parameters U, tau_psc, tau_fac, and tau_rec are common to
all synapses of the model and must be set using SetDefaults on the synapse
model.

References
++++++++++

.. [1] Tsodyks M, Uziel A, Markram H (2000). Synchrony generation in recurrent
       networks with frequency-dependent synapses. Journal of Neuroscience,
       20 RC50. URL: http://infoscience.epfl.ch/record/183402

Transmits
+++++++++

SpikeEvent

See also
++++++++

tsodyks_synapse, stdp_synapse_hom, static_synapse_hom_w

EndUserDocs */

/**
 * Class containing the common properties for all synapses of type
 * TsodyksConnectionHom.
 */

class TsodyksHomCommonProperties : public CommonPropertiesHomW
{

public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  TsodyksHomCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  double tau_psc_; //!< [ms] time constant of postsyn current
  double tau_fac_; //!< [ms] time constant for fascilitation
  double tau_rec_; //!< [ms] time constant for recovery
  double U_;       //!< asymptotic value of probability of release
};


template < typename targetidentifierT >
class TsodyksConnectionHom : public Connection< targetidentifierT >
{
public:
  typedef TsodyksHomCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  TsodyksConnectionHom();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  TsodyksConnectionHom( const TsodyksConnectionHom& );

  /**
   * Default Destructor.
   */
  ~TsodyksConnectionHom()
  {
  }

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
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
  void send( Event& e, thread t, const TsodyksHomCommonProperties& cp );

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
  check_connection( Node& s, Node& t, rport receptor_type, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  void
  set_weight( double )
  {
    throw BadProperty(
      "Setting of individual weights is not possible! The common weights can "
      "be changed via "
      "CopyModel()." );
  }

private:
  double x_;           //!< amount of resources in recovered state
  double y_;           //!< amount of resources in active state
  double u_;           //!< actual probability of release
  double t_lastspike_; //!< time point of last spike emitted
};


/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 */
template < typename targetidentifierT >
inline void
TsodyksConnectionHom< targetidentifierT >::send( Event& e, thread t, const TsodyksHomCommonProperties& cp )
{
  const double t_spike = e.get_stamp().get_ms();
  const double h = t_spike - t_lastspike_;

  // t_lastspike_ = 0 initially
  // this has no influence on the dynamics, IF y = z = 0 initially
  // !!! x != 1.0 -> z != 0.0 -> t_lastspike_=0 has influence on dynamics

  // propagator
  // TODO: use expm1 here instead, where applicable
  double Puu = ( cp.tau_fac_ == 0.0 ) ? 0.0 : std::exp( -h / cp.tau_fac_ );
  double Pyy = std::exp( -h / cp.tau_psc_ );
  double Pzz = std::exp( -h / cp.tau_rec_ );

  double Pxy = ( ( Pzz - 1.0 ) * cp.tau_rec_ - ( Pyy - 1.0 ) * cp.tau_psc_ ) / ( cp.tau_psc_ - cp.tau_rec_ );
  double Pxz = 1.0 - Pzz;

  double z = 1.0 - x_ - y_;

  // propagation t_lastspike_ -> t_spike
  // don't change the order !

  u_ *= Puu;
  x_ += Pxy * y_ + Pxz * z;
  y_ *= Pyy;

  // delta function u
  u_ += cp.U_ * ( 1.0 - u_ );

  // postsynaptic current step caused by incoming spike
  double delta_y_tsp = u_ * x_;

  // delta function x, y
  x_ -= delta_y_tsp;
  y_ += delta_y_tsp;


  e.set_receiver( *get_target( t ) );
  e.set_weight( delta_y_tsp * cp.get_weight() );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  t_lastspike_ = t_spike;
}

template < typename targetidentifierT >
TsodyksConnectionHom< targetidentifierT >::TsodyksConnectionHom()
  : ConnectionBase()
  , x_( 1.0 )
  , y_( 0.0 )
  , u_( 0.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
TsodyksConnectionHom< targetidentifierT >::TsodyksConnectionHom( const TsodyksConnectionHom& rhs )
  : ConnectionBase( rhs )
  , x_( rhs.x_ )
  , y_( rhs.y_ )
  , u_( rhs.u_ )
  , t_lastspike_( rhs.t_lastspike_ )
{
}

template < typename targetidentifierT >
void
TsodyksConnectionHom< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );

  def< double >( d, names::x, x_ );
  def< double >( d, names::y, y_ );
  def< double >( d, names::u, u_ );
}

template < typename targetidentifierT >
void
TsodyksConnectionHom< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // Handle parameters that may throw an exception first, so we can leave the
  // synapse untouched in case of invalid parameter values
  double x = x_;
  double y = y_;
  updateValue< double >( d, names::x, x );
  updateValue< double >( d, names::y, y );

  if ( x + y > 1.0 )
  {
    throw BadProperty( "x + y must be <= 1.0." );
  }

  x_ = x;
  y_ = y;

  ConnectionBase::set_status( d, cm );

  updateValue< double >( d, names::u, u_ );
}

} // namespace

#endif // TSODYKS_CONNECTION_HOM_H
