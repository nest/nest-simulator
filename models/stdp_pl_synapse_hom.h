/*
 *  stdp_pl_synapse_hom.h
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

#ifndef STDP_PL_SYNAPSE_HOM_H
#define STDP_PL_SYNAPSE_HOM_H

// C++ includes:
#include <cmath>
#include <iomanip>

// Includes from nestkernel:
#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, spike-timing-dependent plasticity

Short description
+++++++++++++++++

Synapse type for spike-timing dependent plasticity with power law

Description
+++++++++++

``stdp_pl_synapse`` is a connector to create synapses with spike time
dependent plasticity using homoegeneous parameters (as defined in [1]_).

Parameters
++++++++++

=========  ======  ====================================================
 tau_plus  ms      Time constant of STDP window, potentiation
                   (tau_minus defined in postsynaptic neuron)
 lambda    real    Learning rate
 alpha     real    Asymmetry parameter (scales depressing increments as
                   alpha*lambda)
 mu        real    Weight dependence exponent, potentiation
=========  ======  ====================================================

The parameters can only be set by SetDefaults and apply to all synapses of
the model.

.. warning::

   This synaptic plasticity rule does not take
   :ref:`precise spike timing <sim_precise_spike_times>` into
   account. When calculating the weight update, the precise spike time part
   of the timestamp is ignored.

References
++++++++++

.. [1] Morrison A, Aertsen A, Diesmann M. (2007) Spike-timing dependent
       plasticity in balanced random netrks. Neural Computation,
       19(6):1437-1467. DOI: https://doi.org/10.1162/neco.2007.19.6.1437

Transmits
+++++++++

SpikeEvent

See also
++++++++

stdp_synapse, tsodyks_synapse, static_synapse

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: stdp_pl_synapse_hom

EndUserDocs */

/**
 * Class containing the common properties for all synapses of type
 * stdp_pl_synapse_hom.
 */
class STDPPLHomCommonProperties : public CommonSynapseProperties
{

public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  STDPPLHomCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void init_exp_tau_plus();
  void init_exp_tau_minus();

  double get_exp_tau_plus( const long dt_steps ) const;
  double get_exp_tau_minus( const long dt_steps ) const;

  // data members common to all connections
  double tau_plus_;
  double tau_minus_;
  double minus_tau_plus_inv_;  //!< - 1 / tau_plus for efficiency
  double minus_tau_minus_inv_; //!< - 1 / tau_minus for efficiency
  double lambda_;
  double alpha_;
  double mu_;

  // look up table for the exponentials
  // exp( -dt / tau_plus ) and exp( -dt / tau_minus )
  std::vector< double > exp_tau_plus_;
  std::vector< double > exp_tau_minus_;
};

inline double
STDPPLHomCommonProperties::get_exp_tau_plus( const long dt_steps ) const
{
  if ( static_cast< size_t >( dt_steps ) < exp_tau_plus_.size() )
  {
    return exp_tau_plus_[ dt_steps ];
  }
  else
  {
    return std::exp( Time( Time::step( dt_steps ) ).get_ms() * minus_tau_plus_inv_ );
  }
}

inline double
STDPPLHomCommonProperties::get_exp_tau_minus( const long dt_steps ) const
{
  if ( static_cast< size_t >( dt_steps ) < exp_tau_minus_.size() )
  {
    return exp_tau_minus_[ dt_steps ];
  }
  else
  {
    return std::exp( Time( Time::step( dt_steps ) ).get_ms() * minus_tau_minus_inv_ );
  }
}


/**
 * Class representing an STDP connection with homogeneous parameters, i.e.
 * parameters are the same for all synapses.
 */
void register_stdp_pl_synapse_hom( const std::string& name );

template < typename targetidentifierT >
class stdp_pl_synapse_hom : public Connection< targetidentifierT >
{

public:
  typedef STDPPLHomCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::SUPPORTS_HPC
    | ConnectionModelProperties::SUPPORTS_LBL;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  stdp_pl_synapse_hom();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  stdp_pl_synapse_hom( const stdp_pl_synapse_hom& ) = default;
  stdp_pl_synapse_hom& operator=( const stdp_pl_synapse_hom& ) = default;

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay;
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
   */
  bool send( Event& e, size_t t, const STDPPLHomCommonProperties& );

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
  };

  /*
   * This function calls check_connection on the sender and checks if the
   * receiver accepts the event type and receptor type requested by the sender.
   * Node::check_connection() will either confirm the receiver port by returning
   * true or false if the connection should be ignored.
   * We have to override the base class' implementation, since for STDP
   * connections we have to call register_stdp_pl_connection on the target
   * neuron to inform the Archiver to collect spikes for this connection.
   *
   * \param s The source node
   * \param r The target node
   * \param receptor_type The ID of the requested receptor type
   */
  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& cp )
  {
    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    t.register_stdp_connection( t_lastspike_ - get_delay_steps(), get_delay_steps(), cp.tau_minus_ );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double
  facilitate_( double w, double kplus, const STDPPLHomCommonProperties& cp )
  {
    return w + ( cp.lambda_ * std::pow( w, cp.mu_ ) * kplus );
  }

  double
  depress_( double w, double kminus, const STDPPLHomCommonProperties& cp )
  {
    double new_w = w - ( cp.lambda_ * cp.alpha_ * w * kminus );
    return new_w > 0.0 ? new_w : 0.0;
  }

  // data members of each connection
  double weight_;
  double Kplus_;
  long t_lastspike_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties stdp_pl_synapse_hom< targetidentifierT >::properties;

//
// Implementation of class stdp_pl_synapse_hom.
//

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 */
template < typename targetidentifierT >
inline bool
stdp_pl_synapse_hom< targetidentifierT >::send( Event& e, size_t t, const STDPPLHomCommonProperties& cp )
{
  // synapse STDP depressing/facilitation dynamics

  const long t_spike = e.get_stamp().get_steps();

  // t_lastspike_ = 0 initially

  Node* target = get_target( t );

  const long dendritic_delay = get_delay_steps();

  // get spike history in relevant range (t1, t2] from postsynaptic neuron
  std::deque< histentry_step >::iterator start;
  std::deque< histentry_step >::iterator finish;
  target->get_history( t_lastspike_ - dendritic_delay, t_spike - dendritic_delay, &start, &finish );

  // facilitation due to postsynaptic spikes since last pre-synaptic spike
  size_t dt;
  while ( start != finish )
  {
    dt = ( start->t_ + dendritic_delay ) - t_lastspike_;
    start++;
    // get_history() should make sure that
    // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0

    weight_ = facilitate_( weight_, Kplus_ * cp.get_exp_tau_plus( dt ), cp );
  }

  // depression due to new pre-synaptic spike

  const auto k_val = target->get_K_value( t_spike - dendritic_delay, dt );
  weight_ = depress_( weight_, k_val * cp.get_exp_tau_minus( dt ), cp );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  Kplus_ = Kplus_ * cp.get_exp_tau_plus( t_spike - t_lastspike_ ) + 1.0;

  t_lastspike_ = t_spike;

  return true;
}

template < typename targetidentifierT >
stdp_pl_synapse_hom< targetidentifierT >::stdp_pl_synapse_hom()
  : ConnectionBase()
  , weight_( 1.0 )
  , Kplus_( 0.0 )
  , t_lastspike_( 0 )
{
}

template < typename targetidentifierT >
void
stdp_pl_synapse_hom< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  // base class properties, different for individual synapse
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );

  // own properties, different for individual synapse
  def< double >( d, names::Kplus, Kplus_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
stdp_pl_synapse_hom< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // base class properties
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::Kplus, Kplus_ );
}

} // of namespace nest

#endif // of #ifndef STDP_PL_SYNAPSE_HOM_H
