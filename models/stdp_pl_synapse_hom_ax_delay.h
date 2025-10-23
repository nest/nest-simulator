/*
 *  stdp_pl_synapse_hom_ax_delay.h
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

#ifndef STDP_PL_SYNAPSE_HOM_AX_DELAY_H
#define STDP_PL_SYNAPSE_HOM_AX_DELAY_H

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, spike-timing-dependent plasticity

Short description
+++++++++++++++++

Synapse type for spike-timing dependent plasticity with power law and both dendritic and axonal delays

Description
+++++++++++

``stdp_pl_synapse_hom_ax_delay`` is a connector to create synapses with spike time dependent plasticity using
 homogeneous parameters (as defined in [1]_). Both axonal and dendritic delays can be specified for this model.

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

EndUserDocs */

/**
 * Class containing the common properties for all synapses of type
 * stdp_pl_synapse_hom_ax_delay.
 */
class STDPPLHomAxDelayCommonProperties : public CommonSynapseProperties
{

public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  STDPPLHomAxDelayCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  // data members common to all connections
  double tau_plus_;
  double tau_plus_inv_; //!< 1 / tau_plus for efficiency
  double lambda_;
  double alpha_;
  double mu_;
};

/**
 * Class representing an STDP connection with homogeneous parameters, i.e. parameters are the same for all synapses.
 */
void register_stdp_pl_synapse_hom_ax_delay( const std::string& name );

/**
 * Class representing an STDP connection with homogeneous parameters, i.e.
 * parameters are the same for all synapses.
 */
template < typename targetidentifierT >
class stdp_pl_synapse_hom_ax_delay : public Connection< targetidentifierT, AxonalDendriticDelay >
{

public:
  typedef STDPPLHomAxDelayCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT, AxonalDendriticDelay > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::SUPPORTS_HPC
    | ConnectionModelProperties::SUPPORTS_LBL;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  stdp_pl_synapse_hom_ax_delay();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  stdp_pl_synapse_hom_ax_delay( const stdp_pl_synapse_hom_ax_delay& ) = default;
  stdp_pl_synapse_hom_ax_delay& operator=( const stdp_pl_synapse_hom_ax_delay& ) = default;

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_axonal_delay_ms;
  using ConnectionBase::get_axonal_delay_steps;
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_dendritic_delay_ms;
  using ConnectionBase::get_dendritic_delay_steps;
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
  bool send( Event& e, size_t t, const STDPPLHomAxDelayCommonProperties& );

  /**
   * Framework for STDP with predominantly axonal delays:
   * Correct this synapse and the corresponding previously sent spike
   * taking into account a new post-synaptic spike.
   */
  void correct_synapse_stdp_ax_delay( const size_t tid,
    const size_t lcid,
    const double t_last_spike,
    const double t_spike_critical_interval_end,
    const double weight_revert,
    double& new_weight,
    const double K_plus_revert,
    const double t_post_spike,
    const STDPPLHomAxDelayCommonProperties& cp );

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
  check_connection( Node& s, Node& t, const size_t receptor_type, const synindex syn_id, const CommonPropertiesType& )
  {
    if ( kernel().sp_manager.is_structural_plasticity_enabled() )
    {
      throw IllegalConnection( "Structural plasticity is not supported in combination with axonal delays." );
    }

    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, syn_id, receptor_type );

    if ( get_axonal_delay_ms() + get_dendritic_delay_ms() < kernel().connection_manager.get_stdp_eps() )
    {
      throw BadProperty( "Combination of axonal and dendritic delay has to be more than 0." );
    }
    t.register_stdp_connection( t_lastspike_ - get_dendritic_delay_ms() + get_axonal_delay_ms(),
      get_dendritic_delay_ms(),
      get_axonal_delay_ms() );

    if ( get_axonal_delay_ms() >= get_dendritic_delay_ms() )
    {
      CorrectionSpikeEvent e;
      t.handles_test_event( e, receptor_type );
    }

    // The last spike reference value must resemble a spike that arrived at the synapse at t=0
    t_lastspike_ = -get_axonal_delay_ms();
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double
  facilitate_( double w, double kplus, const STDPPLHomAxDelayCommonProperties& cp )
  {
    return w + ( cp.lambda_ * std::pow( w, cp.mu_ ) * kplus );
  }

  double
  depress_( double w, double kminus, const STDPPLHomAxDelayCommonProperties& cp )
  {
    double new_w = w - ( cp.lambda_ * cp.alpha_ * w * kminus );
    return new_w > 0.0 ? new_w : 0.0;
  }

  // data members of each connection
  double weight_;
  double Kplus_;
  double t_lastspike_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties stdp_pl_synapse_hom_ax_delay< targetidentifierT >::properties;

//! Send an event to the receiver of this connection.
template < typename targetidentifierT >
inline bool
stdp_pl_synapse_hom_ax_delay< targetidentifierT >::send( Event& e,
  size_t tid,
  const STDPPLHomAxDelayCommonProperties& cp )
{
  // synapse STDP depressing/facilitation dynamics
  const double axonal_delay_ms = get_axonal_delay_ms();
  const double dendritic_delay_ms = get_dendritic_delay_ms();
  const double t_spike = e.get_stamp().get_ms();

  // t_lastspike_ = 0 initially
  Node* target = get_target( tid );

  // get spike history in relevant range (t1, t2] from postsynaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_lastspike_ - dendritic_delay_ms + axonal_delay_ms,
    t_spike - dendritic_delay_ms + axonal_delay_ms,
    &start,
    &finish );

  // Framework for STDP with predominantly axonal delays:
  // Store pre-synaptic trace for potential later correction
  const double K_plus_revert = Kplus_;

  // facilitation due to postsynaptic spikes since last pre-synaptic spike
  double minus_dt = 0.;
  while ( start != finish )
  {
    minus_dt = t_lastspike_ + axonal_delay_ms - ( start->t_ + dendritic_delay_ms );
    // get_history() should make sure that
    // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0
    assert( minus_dt < -1.0 * kernel().connection_manager.get_stdp_eps() );
    weight_ = facilitate_( weight_, Kplus_ * std::exp( minus_dt * cp.tau_plus_inv_ ), cp );
    ++start;
  }

  // Framework for STDP with predominantly axonal delays:
  // Store weight before depression for potential later correction
  const double weight_revert = weight_;

  // depression due to new pre-synaptic spike
  const double K_minus = target->get_K_value( t_spike + axonal_delay_ms - dendritic_delay_ms );
  weight_ = depress_( weight_, K_minus, cp );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  // axonal_delay-dendritic_delay = total_delay-2*dendritic_delay
  const long time_while_critical =
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) - 2 * get_dendritic_delay_steps() + 1;
  // Only add correction entry if there could potentially be any post-synaptic spike that occurs before the
  // pre-synaptic one arrives at the synapse.
  if ( time_while_critical > 0 )
  {
    static_cast< ArchivingNode* >( target )->add_correction_entry_stdp_ax_delay(
      static_cast< SpikeEvent& >( e ), t_lastspike_, weight_revert, weight_, K_plus_revert, time_while_critical );
  }

  Kplus_ = Kplus_ * std::exp( ( t_lastspike_ - t_spike ) * cp.tau_plus_inv_ ) + 1.0;

  t_lastspike_ = t_spike;

  return true;
}

template < typename targetidentifierT >
stdp_pl_synapse_hom_ax_delay< targetidentifierT >::stdp_pl_synapse_hom_ax_delay()
  : ConnectionBase()
  , weight_( 1.0 )
  , Kplus_( 0.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
void
stdp_pl_synapse_hom_ax_delay< targetidentifierT >::get_status( DictionaryDatum& d ) const
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
stdp_pl_synapse_hom_ax_delay< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, names::Kplus, Kplus_ );
}

template < typename targetidentifierT >
void
stdp_pl_synapse_hom_ax_delay< targetidentifierT >::correct_synapse_stdp_ax_delay( const size_t tid,
  const size_t lcid,
  const double t_last_spike,
  const double t_spike_critical_interval_end,
  const double weight_revert,
  double& new_weight,
  const double K_plus_revert,
  const double t_post_spike,
  const STDPPLHomAxDelayCommonProperties& cp )
{
  const double wrong_weight = weight_; // incorrectly transmitted weight
  Node* target = get_target( tid );

  const double axonal_delay_ms = get_axonal_delay_ms();
  double dendritic_delay_ms = get_dendritic_delay_ms();

  const double t_spike = t_spike_critical_interval_end + dendritic_delay_ms - axonal_delay_ms;

  // facilitation due to new post-synaptic spike
  const double minus_dt = t_last_spike + axonal_delay_ms - ( t_post_spike + dendritic_delay_ms );

  // Only facilitate if not facilitated already (only if first correction for this post-spike)
  if ( minus_dt < -1.0 * kernel().connection_manager.get_stdp_eps() )
  {
    weight_ = facilitate_( weight_revert, K_plus_revert * std::exp( minus_dt * cp.tau_plus_inv_ ), cp );

    // update weight_revert in case further correction will be required later
    static_cast< ArchivingNode* >( target )->update_weight_revert( lcid, weight_ );
  }

  // depression taking into account new post-synaptic spike
  const double K_minus = target->get_K_value( t_spike + axonal_delay_ms - dendritic_delay_ms );
  weight_ = depress_( weight_, K_minus, cp );

  new_weight = weight_;

  // send a correcting event to the target neuron
  CorrectionSpikeEvent e;
  e.set_receiver( *target );
  e.set_weight( wrong_weight );
  e.set_new_weight( new_weight );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e.set_stamp( Time::ms_stamp( t_spike ) );
  e();
}

} // of namespace nest

#endif // of #ifndef STDP_PL_SYNAPSE_HOM_AX_DELAY_H
