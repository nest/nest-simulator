/*
 *  eprop_readout.h
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

#ifndef EPROP_READOUT_H
#define EPROP_READOUT_H

// nestkernel
#include "connection.h"
#include "eprop_archiving_node.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, e-prop plasticity, current-based

Short description
+++++++++++++++++

Current-based leaky integrate readout neuron model with delta-shaped
postsynaptic currents for e-prop plasticity

Description
+++++++++++

``eprop_readout`` is an implementation of a integrate-and-fire neuron model
with delta-shaped postsynaptic currents used as readout neuron for e-prop plasticity [1]_.
An additional state variable and the corresponding differential
equation represents a piecewise constant external current.

.. math::
    v_j^t &= \alpha v_j^{t-1}+\sum_{i \neq j}W_{ji}^\mathrm{rec}z_i^{t-1}
              + \sum_i W_{ji}^\mathrm{in}x_i^t-z_j^{t-1}v_\mathrm{th} \\
    \alpha &= e^{-\frac{\delta t}{\tau_\mathrm{m}}}

See the documentation on the ``iaf_psc_delta`` neuron model for more information
on the integration of the subthreshold dynamics.

For more information on e-prop plasticity, see the documentation on the other e-prop models:

    * :doc:`eprop_iaf_psc_delta<../models/eprop_iaf_psc_delta/>`
    * :doc:`eprop_iaf_psc_delta_adapt<../models/eprop_iaf_psc_delta_adapt/>`
    * :doc:`eprop_synapse<../models/eprop_synapse/>`
    * :doc:`eprop_learning_signal_connection<../models/eprop_learning_signal_connection/>`

Details on the event-based NEST implementation of e-prop can be found in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================  =======  =======================  ================== ===============================================
**Neuron parameters**
------------------------------------------------------------------------------------------------------------------------
Parameter           Unit     Math equivalent          Default            Description
==================  =======  =======================  ================== ===============================================
 C_m                pF       :math:`C_\text{m}`                    250.0 Capacity of the membrane
 E_L                mV       :math:`E_\text{L}`                      0.0 Leak membrane potential
 I_e                pA       :math:`I_\text{e}`                      0.0 Constant external input current
 loss                        :math:`E`                mean_squared_error Loss function
                                                                         ["mean_squared_error", "cross_entropy_loss"]
 start_learning     ms                                               0.0 Time point to start sending learning signals
 tau_m              ms       :math:`\tau_\text{m}`                  10.0 Time constant of the membrane
 V_m                mV       :math:`v_j^0`                           0.0 Initial value of the membrane voltage
 V_min              mV       :math:`v_\text{min}`             -1.79e+308 Absolute lower value of the membrane voltage
==================  =======  =======================  ================== ===============================================

Recordables
+++++++++++

The following variables can be recorded.

  - error signal ``error_signal``
  - readout signal ``readout_signal``
  - target signal ``target_signal``
  - membrane potential ``V_m``

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

LearningSignalConnectionEvent, DelayedRateConnectionEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DelayedRateConnectionEvent, DataLoggingRequest

See also
++++++++

EndUserDocs */

class eprop_readout : public EpropArchivingNode
{

public:
  eprop_readout();
  eprop_readout( const eprop_readout& );

  using Node::handle;
  using Node::handles_test_event;

  using Node::sends_secondary_event;

  void
  sends_secondary_event( LearningSignalConnectionEvent& )
  {
  }

  void
  sends_secondary_event( DelayedRateConnectionEvent& )
  {
  }

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DelayedRateConnectionEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DelayedRateConnectionEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void pre_run_hook();

  void update( Time const&, const long, const long );

  void compute_error_signal_mean_squared_error( const long& lag );
  void compute_error_signal_cross_entropy_loss( const long& lag );

  void ( eprop_readout::*compute_error_signal )( const long& lag );

  friend class RecordablesMap< eprop_readout >;
  friend class UniversalDataLogger< eprop_readout >;

  struct Parameters_
  {
    double C_m_;            //!< membrane capacitance (pF)
    double E_L_;            //!< leak potential (mV)
    double I_e_;            //!< external DC current (pA)
    std::string loss_;      //!< loss function
    double start_learning_; //!< time point to start sending learning signals
    double tau_m_;          //!< membrane time constant (ms)
    double V_min_;          //!< lower membrane voltage bound relative to leak potential (mV)

    Parameters_();

    void get( DictionaryDatum& ) const;
    double set( const DictionaryDatum&, Node* );
  };

  struct State_
  {
    double error_signal_;   //!< deviation between the readout and target signal
    double readout_signal_; //!< integrated signal read out from the recurrent network
    double target_signal_;  //!< signal the network should learn
    double y0_;             //!< current (pA)
    double y3_;             //!< membrane voltage relative to leak potential (mV)

    State_();

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };

  struct Buffers_
  {
    Buffers_( eprop_readout& );
    Buffers_( const Buffers_&, eprop_readout& );

    RingBuffer delayed_rates_;
    RingBuffer normalization_rates_;

    RingBuffer spikes_;
    RingBuffer currents_;

    UniversalDataLogger< eprop_readout > logger_;
  };

  struct Variables_
  {
    bool in_learning_window_;
    double P30_;
    double P33_; //!< corresponds to kappa in eprop_synapse
    double P33_complement_;
    double readout_signal_;
    double readout_signal_unnorm_;
    bool requires_buffer_;
    long start_learning_step_; //!< time step to start sending learning signals
    double target_signal_;
  };

  /**
   * Minimal spike receptor type.
   * @note Start with 1 so we can forbid port 0 to avoid accidental
   *       creation of connections with no receptor type set.
   */
  static const size_t MIN_RATE_RECEPTOR = 1;

  /**
   * Spike receptors.
   */
  enum RateSynapseTypes
  {
    READOUT_SIG = MIN_RATE_RECEPTOR,
    TARGET_SIG,
    SUP_RATE_RECEPTOR
  };

  static const size_t NUM_RATE_RECEPTORS = SUP_RATE_RECEPTOR - MIN_RATE_RECEPTOR;

  double
  get_V_m_() const
  {
    return S_.y3_ + P_.E_L_;
  }

  double
  get_readout_signal_() const
  {
    return S_.readout_signal_;
  }

  double
  get_target_signal_() const
  {
    return S_.target_signal_;
  }

  double
  get_error_signal_() const
  {
    return S_.error_signal_;
  }

  /**
   * @defgroup eprop_readout_data
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

  static RecordablesMap< eprop_readout > recordablesMap_;
};

inline size_t
eprop_readout::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return 0;
}

inline size_t
eprop_readout::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return 0;
}

inline size_t
eprop_readout::handles_test_event( DelayedRateConnectionEvent& e, size_t receptor_type )
{
  size_t step_rate_model_id = kernel().model_manager.get_node_model_id( "step_rate_generator" );
  size_t model_id = e.get_sender().get_model_id();

  if ( step_rate_model_id == model_id and receptor_type != TARGET_SIG )
    throw IllegalConnection(
      "eprop_readout neurons expect a connection with a step_rate_generator node through receptor_type 2." );

  if ( receptor_type < MIN_RATE_RECEPTOR or receptor_type >= SUP_RATE_RECEPTOR )
    throw UnknownReceptorType( receptor_type, get_name() );

  return receptor_type - MIN_RATE_RECEPTOR;
}

inline size_t
eprop_readout::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_readout::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  DictionaryDatum receptor_dict_ = new Dictionary();
  ( *receptor_dict_ )[ names::readout_signal ] = READOUT_SIG;
  ( *receptor_dict_ )[ names::target_signal ] = TARGET_SIG;

  ( *d )[ names::receptor_types ] = receptor_dict_;
}

inline void
eprop_readout::set_status( const DictionaryDatum& d )
{
  // temporary copies in case of errors
  Parameters_ ptmp = P_;
  State_ stmp = S_;

  // make sure that ptmp and stmp consistent - throw BadProperty if not
  const double delta_EL = ptmp.set( d, this );
  stmp.set( d, ptmp, delta_EL, this );

  P_ = ptmp;
  S_ = stmp;
}

} // namespace nest

#endif // EPROP_READOUT_H
