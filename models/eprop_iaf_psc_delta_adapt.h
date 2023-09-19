/*
 *  eprop_iaf_psc_delta_adapt.h
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

#ifndef EPROP_IAF_PSC_DELTA_ADAPT_H
#define EPROP_IAF_PSC_DELTA_ADAPT_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron model with delta-shaped
postsynaptic currents and threshold adaptation for e-prop plasticity

Description
+++++++++++

``eprop_iaf_psc_delta_adapt`` is an implementation of a leaky integrate-and-fire
neuron model with delta-shaped postsynaptic currents and threshold adaptation
used for eligibility propagation (e-prop) plasticity.

E-prop plasticity was originally introduced and implemented in TensorFlow in [1]_.

The membrane voltage time course is given by:

.. math::
    v_j^t &= \alpha v_j^{t-1}+\sum_{i \neq j}W_{ji}^\mathrm{rec}z_i^{t-1}
             + \sum_i W_{ji}^\mathrm{in}x_i^t-z_j^{t-1}v_\mathrm{th} \,, \\
    \alpha &= e^{-\frac{\delta t}{\tau_\mathrm{m}}} \,.

The threshold adaptation is given by:

.. math::
    A_j^t &= v_\mathrm{th} + \beta a_j^t \,, \\
    a_j^t &= \rho a_j^{t-1} + z_j^{t-1} \,, \\
    \rho &= e^{-\frac{\delta t}{\tau_\mathrm{a}}} \,.

The spike state variable is expressed by a Heaviside function:

.. math::
    z_j^t = H\left(v_j^t-A_j^t\right) \,.

If the membrane voltage crosses the adaptive threshold voltage, a spike is
emitted and the membrane voltage is reduced by :math:`v_\text{th}` in the next
time step. Counted from the time step of the spike emission, the neuron is not
able to spike for an absolute refractory period :math:`t_\text{ref}`.

An additional state variable and the corresponding differential equation
represents a piecewise constant external current.

Furthermore, the pseudo derivative of the membrane voltage needed for e-prop
plasticity is calculated:

.. math::
    \psi_j^t = \frac{\gamma}{v_\text{th}} \text{max}
               \left(0, 1-\left| \frac{v_j^t-A_j^t}{v_\text{th}}\right| \right) \,.

See the documentation on the ``iaf_psc_delta`` neuron model for more information
on the integration of the subthreshold dynamics.

For more information on e-prop plasticity see the documentation on the other e-prop models:

    * :doc:`eprop_iaf_psc_delta<../models/eprop_iaf_psc_delta/>`
    * :doc:`eprop_readout<../models/eprop_readout/>`
    * :doc:`eprop_synapse<../models/eprop_synapse/>`
    * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================  =======  =======================  ==========  ================================================
**Neuron parameters**
------------------------------------------------------------------------------------------------------------------
Parameter           Unit     Math equivalent          Default     Description
==================  =======  =======================  ==========  ================================================
 adapt_beta                  :math:`\beta`                   1.0  Prefactor of the threshold adaptation
 adapt_tau          ms       :math:`\tau_\text{a}`          10.0  Time constant of the threshold adaptation
 adaptation                  :math:`a_j^0`                   0.0  Initial value of the adaptation variable
 C_m                pF       :math:`C_\text{m}`            250.0  Capacity of the membrane
 E_L                mV       :math:`E_\text{L}`            -70.0  Leak membrane potential
 gamma                       :math:`\gamma`                  0.3  Scaling of pseudo-derivative of membrane voltage
 I_e                pA       :math:`I_\text{e}`              0.0  Constant external input current
 t_ref              ms       :math:`t_\text{ref}`            2.0  Duration of the refractory period
 tau_m              ms       :math:`\tau_\text{m}`          20.0  Time constant of the membrane
 V_m                mV       :math:`v_j^0`                 -70.0  Initial value of the membrane voltage
 V_min              mV       :math:`v_\text{min}`     -1.79e+308  Absolute lower value of the membrane voltage
 V_th               mV       :math:`v_\text{th}`           -55.0  Spike threshold
==================  =======  =======================  ==========  ================================================

Recordables
+++++++++++

The following variables can be recorded.

  - ``adaptation``
  - ``adapting_threshold``
  - ``learning_signal``
  - ``V_m``
  - ``V_m_pseudo_deriv``

Usage
+++++

This model can only be used in combination with the other e-prop models,
whereby the network architecture requires specific wiring, input, and output.
The usage is demonstrated in a
:doc:`supervised regression task <../auto_examples/eprop_plasticity/eprop_supervised_regression/>`
and a :doc:`supervised classification task <../auto_examples/eprop_plasticity/eprop_supervised_classification>`,
reproducing the original proof-of-concept tasks in [1]_.

References
++++++++++

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y
.. [2] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D,
       van Albada SJ, Bolten M, Diesmann M. Event-based implementation of
       eligibility propagation (in preparation)

Sends
++++++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, LearningSignalConnectionEvent, DataLoggingRequest

See also
++++++++

EndUserDocs */

class eprop_iaf_psc_delta_adapt : public EpropArchivingNode
{

public:
  eprop_iaf_psc_delta_adapt();
  eprop_iaf_psc_delta_adapt( const eprop_iaf_psc_delta_adapt& );

  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool );

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( LearningSignalConnectionEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( LearningSignalConnectionEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  double get_leak_propagator() const;
  double get_leak_propagator_complement() const;
  double get_adapt_propagator() const;
  double get_adapt_beta() const;

  std::string get_eprop_node_type() const;

private:
  std::string eprop_node_type_ = "adaptive";

  void init_state_( const Node& proto );
  void init_buffers_();
  void pre_run_hook();

  void update( Time const&, const long, const long );

  friend class RecordablesMap< eprop_iaf_psc_delta_adapt >;
  friend class UniversalDataLogger< eprop_iaf_psc_delta_adapt >;

  struct Parameters_
  {
    double tau_m_;      //!< membrane time constant (ms)
    double C_m_;        //!< membrane capacitance (pF)
    double t_ref_;      //!< refractory period (ms)
    double E_L_;        //!< leak potential (mV)
    double I_e_;        //!< external DC current (pA)
    double V_th_;       //!< spike treshold voltage relative to leak potential (mV)
    double V_min_;      //!< lower membrane voltage bound relative to leak potential (mV)
    double adapt_beta_; //!< prefactor of the adaptive threshold voltage
    double adapt_tau_;  //!< time constant of the adaptive threshold (ms)
    double gamma_;      //!< scaling of pseudo-derivative of membrane voltage

    Parameters_();

    void get( DictionaryDatum& ) const;
    double set( const DictionaryDatum&, Node* );
  };

  struct State_
  {
    double y0_;               //!< current (pA)
    double y3_;               //!< membrane voltage relative to leak potential (mV)
    int r_;                   //!< number of remaining refractory steps
    double adaptation_;       //!< adaptation variable
    double V_m_pseudo_deriv_; //!< pseudo derivative of membrane voltage
    double learning_signal_;  //!< weighted error signal

    State_();

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };

  struct Buffers_
  {
    Buffers_( eprop_iaf_psc_delta_adapt& );
    Buffers_( const Buffers_&, eprop_iaf_psc_delta_adapt& );

    RingBuffer spikes_;
    RingBuffer currents_;

    UniversalDataLogger< eprop_iaf_psc_delta_adapt > logger_;
  };

  struct Variables_
  {
    double P30_;
    double P33_;
    double Pa_;
    double P33_complement_;
    bool z_;
    int RefractoryCounts_;
  };

  double
  get_V_m_() const
  {
    return S_.y3_ + P_.E_L_;
  }

  double
  get_V_m_pseudo_deriv_() const
  {
    return S_.V_m_pseudo_deriv_;
  }

  double
  get_learning_signal_() const
  {
    return S_.learning_signal_;
  }

  double
  get_adapting_threshold_() const
  {
    return P_.V_th_ + P_.E_L_ + P_.adapt_beta_ * S_.adaptation_;
  }

  double
  get_adaptation_() const
  {
    return S_.adaptation_;
  }

  /**
   * @defgroup eprop_iaf_psc_delta_adapt_data
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  static RecordablesMap< eprop_iaf_psc_delta_adapt > recordablesMap_;
};

inline size_t
nest::eprop_iaf_psc_delta_adapt::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return 0;
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return 0;
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( LearningSignalConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return 0;
}

inline size_t
eprop_iaf_psc_delta_adapt::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_iaf_psc_delta_adapt::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  EpropArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
eprop_iaf_psc_delta_adapt::set_status( const DictionaryDatum& d )
{
  // temporary copies in case of errors
  Parameters_ ptmp = P_;
  State_ stmp = S_;

  // make sure that ptmp and stmp consistent - throw BadProperty if not
  const double delta_EL = ptmp.set( d, this );
  stmp.set( d, ptmp, delta_EL, this );

  // make sure that properties to be set in parent class consistent
  EpropArchivingNode::set_status( d );

  P_ = ptmp;
  S_ = stmp;
}

} // namespace nest

#endif // EPROP_IAF_PSC_DELTA_ADAPT_H
