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
 C_m                pF       :math:`C_\text{m}`                    250.0 Capacitance of the membrane
 E_L                mV       :math:`E_\text{L}`                      0.0 Leak membrane potential
 I_e                pA       :math:`I_\text{e}`                      0.0 Constant external input current
 loss                        :math:`E`                mean_squared_error Loss function
                                                                         ["mean_squared_error", "cross_entropy_loss"]
 tau_m              ms       :math:`\tau_\text{m}`                  10.0 Time constant of the membrane
 V_m                mV       :math:`v_j^0`                           0.0 Initial value of the membrane voltage
 V_min              mV       :math:`v_\text{min}`             -1.79e+308 Absolute lower bound of the membrane voltage
==================  =======  =======================  ================== ===============================================

Recordables
+++++++++++

The following variables can be recorded:

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

void register_eprop_readout( const std::string& name );

class eprop_readout : public EpropArchivingNode
{

public:
  //! Constructor.
  eprop_readout();

  //! Copy constructor.
  eprop_readout( const eprop_readout& );

  using Node::handle;
  using Node::handles_test_event;

  using Node::sends_secondary_event;

  void
  sends_secondary_event( LearningSignalConnectionEvent& ) override
  {
  }

  void
  sends_secondary_event( DelayedRateConnectionEvent& ) override
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

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  double gradient_change( std::vector< long >& presyn_isis,
    const long t_previous_update,
    const long t_previous_trigger_spike,
    const double kappa,
    const bool average_gradient ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;
  long get_shift() const override;

  void update( Time const&, const long, const long ) override;

  //! Compute the error signal based on the mean-squared error.
  void compute_error_signal_mean_squared_error( const long lag );

  //! Compute the error signal based on the cross-entropy loss.
  void compute_error_signal_cross_entropy_loss( const long lag );

  //! Compute the error signal.
  void ( eprop_readout::*compute_error_signal )( const long lag );

  //! Recordables map.
  friend class RecordablesMap< eprop_readout >;

  //! Universal data logger.
  friend class UniversalDataLogger< eprop_readout >;

  //! Parameters.
  struct Parameters_
  {
    //! Capacitance of the membrane (pF).
    double C_m_;

    //! Leak membrane potential (mV).
    double E_L_;

    //! Constant external input current (pA).
    double I_e_;

    //! Loss function ["mean_squared_error", "cross_entropy_loss"].
    std::string loss_;

    //! Time constant of the membrane (ms).
    double tau_m_;

    //! Absolute lower bound of the membrane voltage relative to the leak membrane potential (mV).
    double V_min_;

    //! Constructor.
    Parameters_();

    //! Get parameters.
    void get( DictionaryDatum& ) const;

    //! Set parameters.
    double set( const DictionaryDatum&, Node* );
  };

  //! State variables.
  struct State_
  {
    //! Deviation between the readout and the target signal.
    double error_signal_;

    //! Readout signal. Leaky integrated spikes emitted by the recurrent network.
    double readout_signal_;

    //! Unnormalized readout signal.
    double readout_signal_unnorm_;

    //! Target signal. The signal the network is supposed to learn.
    double target_signal_;

    //! Input current (pA).
    double y0_;

    //! Membrane voltage relative to the leak membrane potential (mV).
    double y3_;

    //! Constructor.
    State_();

    //! Get the value of the provided parameter from the dictionary.
    void get( DictionaryDatum&, const Parameters_& ) const;

    //! Set the provided parameter in the dictionary to the provided value.
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };

  //! Buffers.
  struct Buffers_
  {
    //! Constructor.
    Buffers_( eprop_readout& );

    //! Copy constructor.
    Buffers_( const Buffers_&, eprop_readout& );

    //! Normalization rate of the readout signal.
    double normalization_rate_;

    //! Buffer for incoming spikes.
    RingBuffer spikes_;

    //! Buffer for incoming currents.
    RingBuffer currents_;

    //! Universal data logger.
    UniversalDataLogger< eprop_readout > logger_;
  };

  struct Variables_
  {
    //! Propagator entry 33.
    double P33_;

    //! Complement of propagator entry 33.
    double P33_complement_;

    //! Propagator entry 30.
    double P30_;

    //! If the loss function requires a buffer for the normalization rates.
    bool requires_buffer_;
  };

  /**
   * Minimal spike receptor type.
   * @note Start with 1 so we can forbid port 0 to avoid accidental
   *       creation of connections with no receptor type set.
   */
  static const size_t MIN_RATE_RECEPTOR = 1;

  //! Spike receptors.
  enum RateSynapseTypes
  {
    READOUT_SIG = MIN_RATE_RECEPTOR,
    TARGET_SIG,
    SUP_RATE_RECEPTOR
  };

  //! Get the membrane voltage.
  double
  get_V_m_() const
  {
    return S_.y3_ + P_.E_L_;
  }

  //! Get the readout signal.
  double
  get_readout_signal_() const
  {
    return S_.readout_signal_;
  }

  //! Get the target signal.
  double
  get_target_signal_() const
  {
    return S_.target_signal_;
  }

  //! Get the error signal.
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
  Parameters_ P_; //!< Parameters.
  State_ S_;      //!< State variables.
  Variables_ V_;  //!< General variables.
  Buffers_ B_;    //!< Buffers.
  /** @} */

  //! Recordables map.
  static RecordablesMap< eprop_readout > recordablesMap_;
};

inline size_t
eprop_readout::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_readout::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_readout::handles_test_event( DelayedRateConnectionEvent& e, size_t receptor_type )
{
  size_t step_rate_model_id = kernel().model_manager.get_node_model_id( "step_rate_generator" );
  size_t model_id = e.get_sender().get_model_id();

  if ( step_rate_model_id == model_id and receptor_type != TARGET_SIG )
  {
    throw IllegalConnection(
      "eprop_readout neurons expect a connection with a step_rate_generator node through receptor_type 2." );
  }

  if ( receptor_type < MIN_RATE_RECEPTOR or receptor_type >= SUP_RATE_RECEPTOR )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return receptor_type;
}

inline size_t
eprop_readout::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

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
