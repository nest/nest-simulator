/*
 *  eprop_learning_signal_connection_bsshslm_2020.h
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


#ifndef EPROP_LEARNING_SIGNAL_CONNECTION_BSSHSLM_2020_H
#define EPROP_LEARNING_SIGNAL_CONNECTION_BSSHSLM_2020_H

// nestkernel
#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, e-prop plasticity

Short description
+++++++++++++++++

Synapse model transmitting feedback learning signals for e-prop plasticity

Description
+++++++++++

``eprop_learning_signal_connection_bsshslm_2020`` is an implementation of a feedback connector from
``eprop_readout_bsshslm_2020`` readout neurons to ``eprop_iaf_bsshslm_2020`` or ``eprop_iaf_adapt_bsshslm_2020``
recurrent neurons that transmits the learning signals :math:`L_j^t` for eligibility propagation (e-prop) plasticity and
has a static weight :math:`B_{jk}`.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The suffix ``_bsshslm_2020`` follows the NEST convention to indicate in the
model name the paper that introduced it by the first letter of the authors' last
names and the publication year.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

 * :doc:`eprop_iaf_bsshslm_2020<../models/eprop_iaf_bsshslm_2020/>`
 * :doc:`eprop_iaf_adapt_bsshslm_2020<../models/eprop_iaf_adapt_bsshslm_2020/>`
 * :doc:`eprop_readout_bsshslm_2020<../models/eprop_readout_bsshslm_2020/>`
 * :doc:`eprop_synapse_bsshslm_2020<../models/eprop_synapse_bsshslm_2020/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

========== ===== ================ ======= ===============
**Individual synapse parameters**
---------------------------------------------------------
Parameter  Unit  Math equivalent  Default Description
========== ===== ================ ======= ===============
``delay``  ms    :math:`d_{jk}`       1.0 Dendritic delay
``weight`` pA    :math:`B_{jk}`       1.0 Synaptic weight
========== ===== ================ ======= ===============

Recordables
+++++++++++

The following variables can be recorded. Note that since this connection lacks
a plasticity mechanism the weight does not evolve over time.

============== ==== =============== ============= ===============
**Synapse recordables**
-----------------------------------------------------------------
State variable Unit Math equivalent Initial value Description
============== ==== =============== ============= ===============
``weight``     pA   :math:`B_{jk}`            1.0 Synaptic weight
============== ==== =============== ============= ===============

Usage
+++++

This model can only be used in combination with the other e-prop models
and the network architecture requires specific wiring, input, and output.
The usage is demonstrated in several
:doc:`supervised regression and classification tasks <../auto_examples/eprop_plasticity/index>`
reproducing among others the original proof-of-concept tasks in [1]_.

Transmits
+++++++++

LearningSignalConnectionEvent

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

.. listexamples:: eprop_learning_signal_connection_bsshslm_2020

EndUserDocs */

void register_eprop_learning_signal_connection_bsshslm_2020( const std::string& name );

/**
 * @brief Class implementing a feedback connection model for e-prop plasticity.
 *
 * Class implementing a synapse model transmitting secondary feedback learning signals for e-prop plasticity
 * according to Bellec et al. (2020).
 */
template < typename targetidentifierT >
class eprop_learning_signal_connection_bsshslm_2020 : public Connection< targetidentifierT >
{

public:
  //! Type of the common synapse properties.
  typedef CommonSynapseProperties CommonPropertiesType;

  //! Type of the connection base.
  typedef Connection< targetidentifierT > ConnectionBase;

  //! Properties of the connection model.
  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::HAS_DELAY;

  //! Default constructor.
  eprop_learning_signal_connection_bsshslm_2020()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  //! Get the secondary learning signal event.
  SecondaryEvent* get_secondary_event();

  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  //! Check if the target accepts the event and receptor type requested by the sender.
  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    LearningSignalConnectionEvent ge;

    s.sends_secondary_event( ge );
    ge.set_sender( s );
    Connection< targetidentifierT >::target_.set_rport( t.handles_test_event( ge, receptor_type ) );
    Connection< targetidentifierT >::target_.set_target( &t );
  }

  //! Send the learning signal event.
  bool
  send( Event& e, size_t t, const CommonSynapseProperties& )
  {
    e.set_weight( weight_ );
    e.set_delay_steps( get_delay_steps() );
    e.set_receiver( *get_target( t ) );
    e.set_rport( get_rport() );
    e();
    return true;
  }

  //! Get the model attributes and their values.
  void get_status( DictionaryDatum& d ) const;

  //! Set the values of the model attributes.
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  //! Set the synaptic weight to the provided value.
  void
  set_weight( const double w )
  {
    weight_ = w;
  }

private:
  //! Synaptic weight.
  double weight_;
};

template < typename targetidentifierT >
constexpr ConnectionModelProperties eprop_learning_signal_connection_bsshslm_2020< targetidentifierT >::properties;

template < typename targetidentifierT >
void
eprop_learning_signal_connection_bsshslm_2020< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
void
eprop_learning_signal_connection_bsshslm_2020< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

template < typename targetidentifierT >
SecondaryEvent*
eprop_learning_signal_connection_bsshslm_2020< targetidentifierT >::get_secondary_event()
{
  return new LearningSignalConnectionEvent();
}

} // namespace nest

#endif // EPROP_LEARNING_SIGNAL_CONNECTION_BSSHSLM_2020_H
