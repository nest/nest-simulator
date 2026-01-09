/*
 *  eprop_synapse_bsshslm_2020.h
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

#ifndef EPROP_SYNAPSE_BSSHSLM_2020_H
#define EPROP_SYNAPSE_BSSHSLM_2020_H

// nestkernel
#include "connection.h"
#include "connector_base.h"
#include "eprop_archiving_node_impl.h"
#include "target_identifier.h"
#include "weight_optimizer.h"

namespace nest
{

/* BeginUserDocs: synapse, e-prop plasticity

Short description
+++++++++++++++++

Synapse type for e-prop plasticity

Description
+++++++++++

``eprop_synapse_bsshslm_2020`` is an implementation of a connector model to create synapses between postsynaptic
neurons :math:`j` and presynaptic neurons :math:`i` for eligibility propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The suffix ``_bsshslm_2020`` follows the NEST convention to indicate in the
model name the paper that introduced it by the first letter of the authors' last
names and the publication year.

The e-prop synapse collects the presynaptic spikes needed for calculating the
weight update. When it is time to update, it triggers the calculation of the
gradient which is specific to the post-synaptic neuron and is thus defined there.

Eventually, it optimizes the weight with the specified optimizer.

E-prop synapses require archiving of continuous quantities. Therefore e-prop
synapses can only be connected to neuron models that are capable of
archiving. So far, compatible models are ``eprop_iaf_bsshslm_2020``,
``eprop_iaf_adapt_bsshslm_2020``, and ``eprop_readout_bsshslm_2020``.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf_bsshslm_2020<../models/eprop_iaf_bsshslm_2020/>`
 * :doc:`eprop_iaf_adapt_bsshslm_2020<../models/eprop_iaf_adapt_bsshslm_2020/>`
 * :doc:`eprop_readout_bsshslm_2020<../models/eprop_readout_bsshslm_2020/>`
 * :doc:`eprop_learning_signal_connection_bsshslm_2020<../models/eprop_learning_signal_connection_bsshslm_2020/>`

For more information on the optimizers, see the documentation of the weight optimizer:

 * :doc:`weight_optimizer<../models/weight_optimizer/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

.. warning::

   This synaptic plasticity rule does not take
   :ref:`precise spike timing <sim_precise_spike_times>` into
   account. When calculating the weight update, the precise spike time part
   of the timestamp is ignored.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================== ======= =============== ========= ======================================================
**Common e-prop synapse parameters**
-------------------------------------------------------------------------------------------------------------
Parameter            Unit    Math equivalent Default   Description
==================== ======= =============== ========= ======================================================
``average_gradient`` Boolean                 ``False`` If ``True``, average the gradient over the learning
                                                       window
``optimizer``                                     {}   Dictionary of optimizer parameters
==================== ======= =============== ========= ======================================================

============= ==== ========================= ======= ================================
**Individual synapse parameters**
-------------------------------------------------------------------------------------
Parameter     Unit Math equivalent           Default Description
============= ==== ========================= ======= ================================
``delay``     ms   :math:`d_{ji}`                1.0 Dendritic delay
``weight``    pA   :math:`W_{ji}`                1.0 Initial value of synaptic weight
============= ==== ========================= ======= ================================

================= ==== ========================= ======= ==============================
**Individual e-prop synapse parameters**
---------------------------------------------------------------------------------------
Parameter         Unit Math equivalent           Default Description
================= ==== ========================= ======= ==============================
``tau_m_readout`` ms   :math:`\tau_\text{m,out}`    10.0 Time constant for low-pass
                                                         filtering of eligibility trace
================= ==== ========================= ======= ==============================

Recordables
+++++++++++

The following variables can be recorded.

================== ==== =============== ============= ===============
**Synapse recordables**
---------------------------------------------------------------------
State variable     Unit Math equivalent Initial value Description
================== ==== =============== ============= ===============
``weight``         pA   :math:`B_{jk}`            1.0 Synaptic weight
================== ==== =============== ============= ===============

Usage
+++++

This model can only be used in combination with the other e-prop models
and the network architecture requires specific wiring, input, and output.
The usage is demonstrated in several
:doc:`supervised regression and classification tasks <../auto_examples/eprop_plasticity/index>`
reproducing among others the original proof-of-concept tasks in [1]_.

Transmits
+++++++++

SpikeEvent, DSSpikeEvent

References
++++++++++

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [2] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Plesser HE,
       Dahmen D, Bolten M, Van Albada SJ, Diesmann M. Event-based
       implementation of eligibility propagation (in preparation)

See also
++++++++

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: eprop_synapse_bsshslm_2020

EndUserDocs */

/**
 * @brief Base class implementing common properties for e-prop synapses.
 *
 * Base class implementing common properties for the e-prop synapse model according to Bellec et al. (2020).
 *
 * This class in particular manages a pointer to weight-optimizer common properties to support
 * exchanging the weight optimizer at runtime. Setting the weight-optimizer common properties
 * determines the WO type. It can only be exchanged as long as no synapses for the model exist.
 * The WO CP object is responsible for providing individual optimizer objects to synapses upon
 * connection.
 *
 * @see WeightOptimizerCommonProperties
 */
class EpropSynapseBSSHSLM2020CommonProperties : public CommonSynapseProperties
{
public:
  // Default constructor.
  EpropSynapseBSSHSLM2020CommonProperties();

  //! Copy constructor.
  EpropSynapseBSSHSLM2020CommonProperties( const EpropSynapseBSSHSLM2020CommonProperties& );

  //! Assignment operator.
  EpropSynapseBSSHSLM2020CommonProperties& operator=( const EpropSynapseBSSHSLM2020CommonProperties& ) = delete;

  //! Destructor.
  ~EpropSynapseBSSHSLM2020CommonProperties();

  //! Get parameter dictionary.
  void get_status( DictionaryDatum& d ) const;

  //! Update values in parameter dictionary.
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  //! If True, average the gradient over the learning window.
  bool average_gradient_;

  /**
   * Pointer to common properties object for weight optimizer.
   *
   * @note Must only be changed as long as no synapses of the model exist.
   */
  WeightOptimizerCommonProperties* optimizer_cp_;
};

//! Register the eprop synapse model.
void register_eprop_synapse_bsshslm_2020( const std::string& name );

/**
 * @brief Class implementing a synapse model for e-prop plasticity.
 *
 * Class implementing a synapse model for e-prop plasticity according to Bellec et al. (2020).
 *
 * @note Several aspects of this synapse are in place to reproduce the Tensorflow implementation of Bellec et al.
 * (2020).
 *
 * @note Each synapse has a optimizer_ object managed through a `WeightOptimizer*`, pointing to an object of
 * a specific weight optimizer type. This optimizer, drawing also on parameters in the `WeightOptimizerCommonProperties`
 * accessible via the synapse models `CommonProperties::optimizer_cp_` pointer, computes the weight update for the
 * neuron. The actual optimizer type can be selected at runtime (before creating any synapses) by exchanging the
 * `optimizer_cp_` pointer. Individual optimizer objects are created by `check_connection()` when a synapse is actually
 * created. It is important that the constructors of `eprop_synapse_bsshslm_2020` **do not** create optimizer objects
 * and that the destructor **does not** delete optimizer objects; this currently leads to bugs when using Boosts's
 * `spreadsort()` due to use of the copy constructor where it should suffice to use the move constructor. Therefore,
 * `check_connection()`creates the optimizer object when it is needed and specializations of `Connector::~Connector()`
 * and `Connector::disable_connection()` delete it by calling `delete_optimizer()`. A disadvantage of this approach is
 * that the `default_connection` in the connector model does not have an optimizer object, whence it is not possible to
 * set default (initial) values for the per-synapse optimizer.
 *
 * @note If we can find a way to modify our co-sorting of source and target tables in Boost's `spreadsort()` to only use
 * move operations, it should be possible to create the individual optimizers in the copy constructor of
 * `eprop_synapse_bsshslm_2020` and to delete it in the destructor. The `default_connection` can then own an optimizer
 * and default values could be set on it.
 */
template < typename targetidentifierT >
class eprop_synapse_bsshslm_2020 : public Connection< targetidentifierT >
{

public:
  //! Type of the common synapse properties.
  typedef EpropSynapseBSSHSLM2020CommonProperties CommonPropertiesType;

  //! Type of the connection base.
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Properties of the connection model.
   *
   * @note Does not support LBL at present because we cannot properly cast GenericModel common props in that case.
   */
  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::REQUIRES_EPROP_ARCHIVING
    | ConnectionModelProperties::SUPPORTS_HPC;

  //! Default constructor.
  eprop_synapse_bsshslm_2020();

  //! Destructor
  ~eprop_synapse_bsshslm_2020();

  //! Parameterized copy constructor.
  eprop_synapse_bsshslm_2020( const eprop_synapse_bsshslm_2020& );

  //! Assignment operator
  eprop_synapse_bsshslm_2020& operator=( const eprop_synapse_bsshslm_2020& );

  //! Move constructor
  eprop_synapse_bsshslm_2020( eprop_synapse_bsshslm_2020&& );

  //! Move assignment operator
  eprop_synapse_bsshslm_2020& operator=( eprop_synapse_bsshslm_2020&& );

  using ConnectionBase::get_delay;
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  //! Get parameter dictionary.
  void get_status( DictionaryDatum& d ) const;

  //! Update values in parameter dictionary.
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  //! Send the spike event.
  bool send( Event& e, size_t thread, const EpropSynapseBSSHSLM2020CommonProperties& cp );

  //! Dummy node for testing the connection.
  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    using ConnTestDummyNodeBase::handles_test_event;

    size_t
    handles_test_event( SpikeEvent&, size_t )
    {
      return invalid_port;
    }

    size_t
    handles_test_event( DSSpikeEvent&, size_t )
    {
      return invalid_port;
    }
  };

  /**
   * Check if the target accepts the event and receptor type requested by the sender.
   *
   * @note This sets the optimizer_ member.
   */
  void check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& cp );

  //! Set the synaptic weight to the provided value.
  void
  set_weight( const double w )
  {
    weight_ = w;
  }

  //! Delete optimizer
  void delete_optimizer();

private:
  //! Synaptic weight.
  double weight_;

  //! The time step when the previous spike arrived.
  long t_spike_previous_;

  //! The time step when the previous e-prop update was.
  long t_previous_update_;

  //! The time step when the next e-prop update will be.
  long t_next_update_;

  //! The time step when the spike arrived that triggered the previous e-prop update.
  long t_previous_trigger_spike_;

  //! %Time constant for low-pass filtering the eligibility trace.
  double tau_m_readout_;

  //! Low-pass filter of the eligibility trace.
  double kappa_;

  //! If this connection is between two recurrent neurons.
  bool is_recurrent_to_recurrent_conn_;

  //! Vector of presynaptic inter-spike-intervals.
  std::vector< long > presyn_isis_;

  /**
   *  Optimizer
   *
   *  @note Pointer is set by check_connection() and deleted by delete_optimizer().
   */
  WeightOptimizer* optimizer_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties eprop_synapse_bsshslm_2020< targetidentifierT >::properties;

// Explicitly declare specializations of Connector methods that need to do special things for eprop_synapse_bsshslm_2020
template <>
void Connector< eprop_synapse_bsshslm_2020< TargetIdentifierPtrRport > >::disable_connection( const size_t lcid );
template <>
void Connector< eprop_synapse_bsshslm_2020< TargetIdentifierIndex > >::disable_connection( const size_t lcid );
template <>
Connector< eprop_synapse_bsshslm_2020< TargetIdentifierPtrRport > >::~Connector();
template <>
Connector< eprop_synapse_bsshslm_2020< TargetIdentifierIndex > >::~Connector();


template < typename targetidentifierT >
eprop_synapse_bsshslm_2020< targetidentifierT >::eprop_synapse_bsshslm_2020()
  : ConnectionBase()
  , weight_( 1.0 )
  , t_spike_previous_( 0 )
  , t_previous_update_( 0 )
  , t_next_update_( 0 )
  , t_previous_trigger_spike_( 0 )
  , tau_m_readout_( 10.0 )
  , kappa_( std::exp( -Time::get_resolution().get_ms() / tau_m_readout_ ) )
  , is_recurrent_to_recurrent_conn_( false )
  , optimizer_( nullptr )
{
}

template < typename targetidentifierT >
eprop_synapse_bsshslm_2020< targetidentifierT >::~eprop_synapse_bsshslm_2020()
{
}

// This copy constructor is used to create instances from prototypes.
// Therefore, only parameter values are copied.
template < typename targetidentifierT >
eprop_synapse_bsshslm_2020< targetidentifierT >::eprop_synapse_bsshslm_2020( const eprop_synapse_bsshslm_2020& es )
  : ConnectionBase( es )
  , weight_( es.weight_ )
  , t_spike_previous_( 0 )
  , t_previous_update_( 0 )
  , t_next_update_( kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps() )
  , t_previous_trigger_spike_( 0 )
  , tau_m_readout_( es.tau_m_readout_ )
  , kappa_( std::exp( -Time::get_resolution().get_ms() / tau_m_readout_ ) )
  , is_recurrent_to_recurrent_conn_( es.is_recurrent_to_recurrent_conn_ )
  , optimizer_( es.optimizer_ )
{
}

// This copy assignment operator is used to write a connection into the connection array.
template < typename targetidentifierT >
eprop_synapse_bsshslm_2020< targetidentifierT >&
eprop_synapse_bsshslm_2020< targetidentifierT >::operator=( const eprop_synapse_bsshslm_2020& es )
{
  if ( this == &es )
  {
    return *this;
  }

  ConnectionBase::operator=( es );

  weight_ = es.weight_;
  t_spike_previous_ = es.t_spike_previous_;
  t_previous_update_ = es.t_previous_update_;
  t_next_update_ = es.t_next_update_;
  t_previous_trigger_spike_ = es.t_previous_trigger_spike_;
  tau_m_readout_ = es.tau_m_readout_;
  kappa_ = es.kappa_;
  is_recurrent_to_recurrent_conn_ = es.is_recurrent_to_recurrent_conn_;
  optimizer_ = es.optimizer_;

  return *this;
}

template < typename targetidentifierT >
eprop_synapse_bsshslm_2020< targetidentifierT >::eprop_synapse_bsshslm_2020( eprop_synapse_bsshslm_2020&& es )
  : ConnectionBase( es )
  , weight_( es.weight_ )
  , t_spike_previous_( 0 )
  , t_previous_update_( 0 )
  , t_next_update_( es.t_next_update_ )
  , t_previous_trigger_spike_( 0 )
  , tau_m_readout_( es.tau_m_readout_ )
  , kappa_( es.kappa_ )
  , is_recurrent_to_recurrent_conn_( es.is_recurrent_to_recurrent_conn_ )
  , optimizer_( es.optimizer_ )
{
  es.optimizer_ = nullptr;
}

// This move assignment operator is used to write a connection into the connection array.
template < typename targetidentifierT >
eprop_synapse_bsshslm_2020< targetidentifierT >&
eprop_synapse_bsshslm_2020< targetidentifierT >::operator=( eprop_synapse_bsshslm_2020&& es )
{
  if ( this == &es )
  {
    return *this;
  }

  ConnectionBase::operator=( es );

  weight_ = es.weight_;
  t_spike_previous_ = es.t_spike_previous_;
  t_previous_update_ = es.t_previous_update_;
  t_next_update_ = es.t_next_update_;
  t_previous_trigger_spike_ = es.t_previous_trigger_spike_;
  tau_m_readout_ = es.tau_m_readout_;
  kappa_ = es.kappa_;
  is_recurrent_to_recurrent_conn_ = es.is_recurrent_to_recurrent_conn_;

  optimizer_ = es.optimizer_;
  es.optimizer_ = nullptr;

  return *this;
}

template < typename targetidentifierT >
inline void
eprop_synapse_bsshslm_2020< targetidentifierT >::check_connection( Node& s,
  Node& t,
  size_t receptor_type,
  const CommonPropertiesType& cp )
{
  // When we get here, delay has been set so we can check it.
  if ( get_delay_steps() != 1 )
  {
    throw IllegalConnection( "eprop synapses currently require a delay of one simulation step" );
  }

  ConnTestDummyNode dummy_target;
  ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

  t.register_eprop_connection();

  optimizer_ = cp.optimizer_cp_->get_optimizer();
}

template < typename targetidentifierT >
inline void
eprop_synapse_bsshslm_2020< targetidentifierT >::delete_optimizer()
{
  delete optimizer_;
  // do not set to nullptr to allow detection of double deletion
}

template < typename targetidentifierT >
bool
eprop_synapse_bsshslm_2020< targetidentifierT >::send( Event& e,
  size_t thread,
  const EpropSynapseBSSHSLM2020CommonProperties& cp )
{
  Node* target = get_target( thread );
  assert( target );

  const long t_spike = e.get_stamp().get_steps();
  const long update_interval = kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps();
  const long shift = target->get_shift();

  const long interval_step = ( t_spike - shift ) % update_interval;

  if ( target->is_eprop_recurrent_node() and interval_step == 0 )
  {
    return false;
  }

  if ( t_previous_trigger_spike_ == 0 )
  {
    t_previous_trigger_spike_ = t_spike;
  }

  if ( t_spike_previous_ > 0 )
  {
    const long t = t_spike >= t_next_update_ + shift ? t_next_update_ + shift : t_spike;
    presyn_isis_.push_back( t - t_spike_previous_ );
  }

  if ( t_spike > t_next_update_ + shift )
  {
    const long idx_current_update = ( t_spike - shift ) / update_interval;
    const long t_current_update = idx_current_update * update_interval;

    target->write_update_to_history( t_previous_update_, t_current_update );

    const double gradient = target->compute_gradient(
      presyn_isis_, t_previous_update_, t_previous_trigger_spike_, kappa_, cp.average_gradient_ );

    weight_ = optimizer_->optimized_weight( *cp.optimizer_cp_, idx_current_update, gradient, weight_ );

    t_previous_update_ = t_current_update;
    t_next_update_ = t_current_update + update_interval;

    t_previous_trigger_spike_ = t_spike;
  }

  t_spike_previous_ = t_spike;

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  return true;
}

template < typename targetidentifierT >
void
eprop_synapse_bsshslm_2020< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::tau_m_readout, tau_m_readout_ );
  def< long >( d, names::size_of, sizeof( *this ) );

  DictionaryDatum optimizer_dict = new Dictionary();

  // The default_connection_ has no optimizer, therefore we need to protect it
  if ( optimizer_ )
  {
    optimizer_->get_status( optimizer_dict );
    ( *d )[ names::optimizer ] = optimizer_dict;
  }
}

template < typename targetidentifierT >
void
eprop_synapse_bsshslm_2020< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  if ( d->known( names::optimizer ) and optimizer_ )
  {
    optimizer_->set_status( getValue< DictionaryDatum >( d->lookup( names::optimizer ) ) );
  }

  updateValue< double >( d, names::weight, weight_ );

  if ( updateValue< double >( d, names::tau_m_readout, tau_m_readout_ ) )
  {
    if ( tau_m_readout_ <= 0 )
    {
      throw BadProperty( "Membrane time constant of readout neuron tau_m_readout > 0 required." );
    }
    kappa_ = std::exp( -Time::get_resolution().get_ms() / tau_m_readout_ );
  }

  const auto& gcm =
    dynamic_cast< const GenericConnectorModel< eprop_synapse_bsshslm_2020< targetidentifierT > >& >( cm );
  const CommonPropertiesType& epcp = gcm.get_common_properties();
  if ( weight_ < epcp.optimizer_cp_->get_Wmin() )
  {
    throw BadProperty( "Minimal weight Wmin ≤ weight required." );
  }

  if ( weight_ > epcp.optimizer_cp_->get_Wmax() )
  {
    throw BadProperty( "weight ≤ maximal weight Wmax required." );
  }
}

} // namespace nest

#endif // EPROP_SYNAPSE_BSSHSLM_2020_H
