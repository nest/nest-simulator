/*
 *  clopath_synapse.h
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

#ifndef CLOPATH_SYNAPSE_H
#define CLOPATH_SYNAPSE_H

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connection.h"
#include "connector_model.h"
#include "event.h"
#include "nest_impl.h"
#include "ring_buffer.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{

/* BeginUserDocs: synapse, spike-timing-dependent plasticity, Clopath plasticity

Short description
+++++++++++++++++

Synapse type for voltage-based STDP after Clopath

Description
+++++++++++

``clopath_synapse`` is a connector to create Clopath synapses as defined
in [1]_. In contrast to usual STDP, the change of the synaptic weight does
not only depend on the pre- and postsynaptic spike timing but also on the
postsynaptic membrane potential.

Clopath synapses require archiving of continuous quantities. Therefore Clopath
synapses can only be connected to neuron models that are capable of doing this
archiving. So far, compatible models are ``aeif_psc_delta_clopath`` and
``hh_psc_alpha_clopath``.

.. warning::

   This synaptic plasticity rule does not take
   :ref:`precise spike timing <sim_precise_spike_times>` into
   account. When calculating the weight update, the precise spike time part
   of the timestamp is ignored.

See also [2]_, [3]_.

Parameters
++++++++++

=======  ======  ==========================================================
tau_x    ms      Time constant of the trace of the presynaptic spike train
Wmax     real    Maximum allowed weight
Wmin     real    Minimum allowed weight
=======  ======  ==========================================================

Other parameters like the amplitudes for long-term potentiation (LTP) and
depression (LTD) are stored in the neuron models that are compatible with the
Clopath synapse.

Transmits
+++++++++

SpikeEvent

References
++++++++++

.. [1] Clopath et al. (2010). Connectivity reflects coding:
       a model of voltage-based STDP with homeostasis.
       Nature Neuroscience 13:3, 344--352. DOI: https://doi.org/10.1038/nn.2479
.. [2] Clopath and Gerstner (2010). Voltage and spike timing interact
       in STDP – a unified model. Frontiers in Synaptic Neuroscience 2:25.
       DOI: https://doi.org/10.3389/fnsyn.2010.00025
.. [3] Voltage-based STDP synapse (Clopath et al. 2010) on ModelDB
       https://modeldb.science/144566?tab=1

See also
++++++++

stdp_synapse, aeif_psc_delta_clopath, hh_psc_alpha_clopath

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_psc_alpha

EndUserDocs */

// connections are templates of target identifier type (used for pointer /
// target index addressing) derived from generic connection template
void register_clopath_synapse( const std::string& name );

template < typename targetidentifierT >
class clopath_synapse : public Connection< targetidentifierT >
{

public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY
    | ConnectionModelProperties::IS_PRIMARY | ConnectionModelProperties::REQUIRES_CLOPATH_ARCHIVING
    | ConnectionModelProperties::SUPPORTS_HPC | ConnectionModelProperties::SUPPORTS_LBL
    | ConnectionModelProperties::SUPPORTS_WFR;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  clopath_synapse();


  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  clopath_synapse( const clopath_synapse& ) = default;
  clopath_synapse& operator=( const clopath_synapse& ) = default;

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
  };

  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;

    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    t.register_stdp_connection( t_lastspike_ - get_delay(), get_delay() );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double
  depress_( double w, double dw )
  {
    w -= dw;
    return w > Wmin_ ? w : Wmin_;
  }

  double
  facilitate_( double w, double dw, double x_bar )
  {
    w += dw * x_bar;
    return w < Wmax_ ? w : Wmax_;
  }

  // data members of each connection
  double weight_;
  double x_bar_;
  double tau_x_;
  double Wmin_;
  double Wmax_;

  double t_lastspike_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties clopath_synapse< targetidentifierT >::properties;

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param cp Common properties object, containing the stdp parameters.
 */
template < typename targetidentifierT >
inline bool
clopath_synapse< targetidentifierT >::send( Event& e, size_t t, const CommonSynapseProperties& )
{
  double t_spike = e.get_stamp().get_ms();
  // use accessor functions (inherited from Connection< >) to obtain delay and
  // target
  Node* target = get_target( t );
  double dendritic_delay = get_delay();

  // get spike history in relevant range (t1, t2] from postsynaptic neuron
  std::deque< histentry_extended >::iterator start;
  std::deque< histentry_extended >::iterator finish;

  // For a new synapse, t_lastspike_ contains the point in time of the last
  // spike. So we initially read the
  // history(t_last_spike - dendritic_delay, ..., T_spike-dendritic_delay]
  // which increases the access counter for these entries.

  // Note that in the STDP synapse, this loop iterates over post spikes,
  // whereas here we loop over continuous-time history entries (see
  // histentry_extended).
  target->get_LTP_history( t_lastspike_ - dendritic_delay, t_spike - dendritic_delay, &start, &finish );
  while ( start != finish )
  {
    const double minus_dt = t_lastspike_ - ( start->t_ + dendritic_delay );

    // facilitation due to postsynaptic activity since last pre-synaptic spike
    weight_ = facilitate_( weight_, start->dw_, x_bar_ * exp( minus_dt / tau_x_ ) );

    ++start;
  }

  // depression due to new pre-synaptic spike
  weight_ = depress_( weight_, target->get_LTD_value( t_spike - dendritic_delay ) );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  // use accessor functions (inherited from Connection< >) to obtain delay in
  // steps and rport
  e.set_delay_steps( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  // compute the trace of the presynaptic spike train
  x_bar_ = x_bar_ * std::exp( ( t_lastspike_ - t_spike ) / tau_x_ ) + 1.0 / tau_x_;

  t_lastspike_ = t_spike;

  return true;
}


template < typename targetidentifierT >
clopath_synapse< targetidentifierT >::clopath_synapse()
  : ConnectionBase()
  , weight_( 1.0 )
  , x_bar_( 0.0 )
  , tau_x_( 15.0 )
  , Wmin_( 0.0 )
  , Wmax_( 100.0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
void
clopath_synapse< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< double >( d, names::x_bar, x_bar_ );
  def< double >( d, names::tau_x, tau_x_ );
  def< double >( d, names::Wmin, Wmin_ );
  def< double >( d, names::Wmax, Wmax_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
clopath_synapse< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
  updateValue< double >( d, names::x_bar, x_bar_ );
  updateValue< double >( d, names::tau_x, tau_x_ );
  updateValue< double >( d, names::Wmin, Wmin_ );
  updateValue< double >( d, names::Wmax, Wmax_ );

  // check if weight_ and Wmin_ has the same sign
  if ( not( ( ( weight_ >= 0 ) - ( weight_ < 0 ) ) == ( ( Wmin_ >= 0 ) - ( Wmin_ < 0 ) ) ) )
  {
    throw BadProperty( "Weight and Wmin must have same sign." );
  }

  // check if weight_ and Wmax_ has the same sign
  if ( not( ( ( weight_ >= 0 ) - ( weight_ < 0 ) ) == ( ( Wmax_ > 0 ) - ( Wmax_ <= 0 ) ) ) )
  {
    throw BadProperty( "Weight and Wmax must have same sign." );
  }
}

} // of namespace nest

#endif // of #ifndef CLOPATH_SYNAPSE_H
