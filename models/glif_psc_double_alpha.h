/*
 *  glif_psc_double_alpha.h
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

#ifndef GLIF_PSC_DOUBLE_ALPHA_H
#define GLIF_PSC_DOUBLE_ALPHA_H

#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

#include "dictdatum.h"

/* BeginUserDocs: neuron, integrate-and-fire, current-based, adaptation, hard threshold

Short description
+++++++++++++++++

Current-based generalized leaky integrate-and-fire (GLIF) models with double alpha-function
(from the Allen Institute)

Description
+++++++++++

``glif_psc_double_alpha`` provides five generalized leaky integrate-and-fire
(GLIF) models [1]_ with double alpha-function shaped synaptic currents.
Incoming spike events induce a postsynaptic change of current modeled
by the sum of two alpha functions (fast and slow components) for each receptor [2]_.
This function is normalized such that an event of weight 1.0 results in a peak current
of the fast component of the alpha function to be 1 pA at
:math:`t = \tau_\text{syn, fast}`.
The relative peak current of the slow component is given as ``amp_slow``, at
:math:`t = \tau_\text{syn, slow}`. Namely,

.. math::

    I_\text{syn} = \text{alpha_function} \left( \tau_\text{syn} = \tau_\text{syn, fast} \right) + \text{amp_slow} \cdot
    \text{alpha_function} \left( \tau_\text{syn} = \tau_\text{syn, slow} \right).

Therefore if ``amp_slow`` is not 0, the peak current of the total synaptic current is larger
than the specified weight. By default, ``glif_psc_double_alpha`` has a single synapse that
is accessible through ``receptor_port`` 1. An arbitrary number of synapses with different
time constants and ``amp_slow`` can be configured by setting the desired parameters of
``tau_syn_fast``, ``tau_syn_slow``, and ``amp_slow`` arrays. The resulting synapses are addressed
through ``receptor_port`` 1, 2, 3, ....

The five GLIF models are:

* **GLIF Model 1** - Traditional leaky integrate and fire (LIF)
* **GLIF Model 2** - Leaky integrate and fire with biologically defined reset rules
  (LIF_R)
* **GLIF Model 3** - Leaky integrate and fire with after-spike currents (LIF_ASC)
* **GLIF Model 4** - Leaky integrate and fire with biologically defined reset rules
  and after-spike currents (LIF_R_ASC)
* **GLIF Model 5** - Leaky integrate and fire with biologically defined reset rules,
  after-spike currents and a voltage dependent threshold (LIF_R_ASC_A)

GLIF model mechanism setting is based on three parameters
(``spike_dependent_threshold``, ``after_spike_currents``, ``adapting_threshold``).
The settings of these three parameters for the five GLIF models are listed
below. Other combinations of these parameters will not be supported.

+--------+---------------------------+----------------------+--------------------+
| Model  | spike_dependent_threshold | after_spike_currents | adapting_threshold |
+========+===========================+======================+====================+
| GLIF1  | False                     | False                | False              |
+--------+---------------------------+----------------------+--------------------+
| GLIF2  | True                      | False                | False              |
+--------+---------------------------+----------------------+--------------------+
| GLIF3  | False                     | True                 | False              |
+--------+---------------------------+----------------------+--------------------+
| GLIF4  | True                      | True                 | False              |
+--------+---------------------------+----------------------+--------------------+
| GLIF5  | True                      | True                 | True               |
+--------+---------------------------+----------------------+--------------------+

Typical parameter setting of different levels of GLIF models for different cells
can be found and downloaded in the `Allen Cell Type Database
<https://celltypes.brain-map.org>`_. For example, the default parameter setting of this
``glif_psc_double_alpha`` neuron model was from the parameter values of GLIF Model 5 of
Cell 490626718, which can be retrieved from the `Allen Brain Atlas
<https://celltypes.brain-map.org/mouse/experiment/electrophysiology/
490626718>`_, with units being converted from SI units (i.e., V, S (1/Ohm),
F, s, A) to NEST used units (i.e., mV, nS (1/GOhm), pF, ms, pA) and values
being rounded to appropriate digits for simplification.

For models with spike dependent threshold (i.e., GLIF2, GLIF4 and GLIF5),
parameter setting of ``voltage_reset_fraction`` and ``voltage_reset_add`` may lead to the
situation that voltage is bigger than threshold after reset. In this case, the neuron
will continue to spike until the end of the simulation regardless the stimulated inputs.
We recommend the setting of the parameters of these three models to follow the
condition of

.. math::

    E_L + \mathrm{voltage\_reset\_fraction} \cdot \left( V_\mathrm{th} - E_L \right)
    + \mathrm{voltage\_reset\_add} < V_\mathrm{th} + \mathrm{th\_spike\_add}

.. note::

  If ``tau_m`` is very close to ``tau_syn_ex`` or ``tau_syn_in``, the model
  will numerically behave as if ``tau_m`` is equal to ``tau_syn_ex`` or
  ``tau_syn_in``, respectively, to avoid numerical instabilities.

  For implementation details see the
  `IAF Integration Singularity <../model_details/IAF_Integration_Singularity.ipynb>`_ notebook.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

========= ======== ============================================================
**Membrane parameters**
-------------------------------------------------------------------------------
V_m        double   Membrane potential in mV (absolute value)
V_th       double   Instantaneous threshold in mV
g          double   Membrane conductance in nS
E_L        double   Resting membrane potential in mV
C_m        double   Capacitance of the membrane in pF
t_ref      double   Duration of refractory time in ms
V_reset    double   Reset potential of the membrane in mV (GLIF 1 or GLIF 3)
========= ======== ============================================================

========================= =============== =====================================
**Spike adaptation and firing intensity parameters**
-------------------------------------------------------------------------------
th_spike_add               double         Threshold addition following spike
                                          in mV (delta_theta_s in Equation (6)
                                          in [1]_)
th_spike_decay             double         Spike-induced threshold time
                                          constant in 1/ms (bs in Equation (2)
                                          in [1]_)
voltage_reset_fraction     double         Voltage fraction coefficient
                                          following spike (fv in Equation (5)
                                          in [1]_)
voltage_reset_add          double         Voltage addition following spike in
                                          mV (-delta_V (sign flipped) in
                                          Equation (5) in [1]_)
asc_init                   double vector  Initial values of after-spike
                                          currents in pA
asc_decay                  double vector  After-spike current time constants
                                          in 1/ms (kj in Equation (3) in [1]_)
asc_amps                   double vector  After-spike current amplitudes in
                                          pA (deltaIj in Equation (7) in [1]_)
asc_r                      double vector  Current fraction following spike
                                          coefficients for fj in Equation (7)
                                          in [1]_
th_voltage_index           double         Adaptation index of threshold - A
                                          'leak-conductance' for the
                                          voltage-dependent component of the
                                          threshold in 1/ms (av in Equation
                                          (4) in [1]_)
th_voltage_decay           double         Voltage-induced threshold time
                                          constant - Inverse of which is the
                                          time constant of the
                                          voltage-dependent component of the
                                          threshold in 1/ms (bv in Equation
                                          (4) in [1]_)
tau_syn_fast               double vector  Time constants of the faster
                                          synaptic alpha function in ms
tau_syn_slow               double vector  Time constants of the slower
                                          synaptic alpha function in ms
amp_slow                   double vector  Relative amplitude of the slower
                                          synaptic alpha function
E_rev                      double vector  Reversal potential in mV
spike_dependent_threshold  bool           flag whether the neuron has
                                          biologically defined reset rules
                                          with a spike dependent threshold
                                          component
after_spike_currents       bool           flag whether the neuron has after
                                          spike currents
adapting_threshold         bool           flag whether the neuron has a
                                          voltage dependent threshold component
========================= =============== =====================================

References
++++++++++

.. [1] Teeter C, Iyer R, Menon V, Gouwens N, Feng D, Berg J, Szafer A,
       Cain N, Zeng H, Hawrylycz M, Koch C, & Mihalas S (2018)
       Generalized leaky integrate-and-fire models classify multiple neuron
       types. Nature Communications 9:709.
.. [2] Meffin, H., Burkitt, A. N., & Grayden, D. B. (2004). An analytical
       model for the large, fluctuating synaptic conductance state typical of
       neocortical neurons in vivo. J.  Comput. Neurosci., 16, 159-175.

See also
++++++++

gif_psc_exp_multisynapse, gif_cond_exp, gif_cond_exp_multisynapse, gif_pop_psc_exp,
glif_psc

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: glif_psc_double_alpha

EndUserDocs */

namespace nest
{

void register_glif_psc_double_alpha( const std::string& name );

class glif_psc_double_alpha : public ArchivingNode
{
public:
  glif_psc_double_alpha();

  glif_psc_double_alpha( const glif_psc_double_alpha& );

  using nest::Node::handle;
  using nest::Node::handles_test_event;

  size_t send_test_event( nest::Node&, size_t, nest::synindex, bool ) override;

  void handle( nest::SpikeEvent& ) override;
  void handle( nest::CurrentEvent& ) override;
  void handle( nest::DataLoggingRequest& ) override;

  size_t handles_test_event( nest::SpikeEvent&, size_t ) override;
  size_t handles_test_event( nest::CurrentEvent&, size_t ) override;
  size_t handles_test_event( nest::DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  //! Reset internal buffers of neuron.
  void init_buffers_() override;

  //! Initialize auxiliary quantities, leave parameters and state untouched.
  void pre_run_hook() override;

  //! Take neuron through given time interval
  void update( nest::Time const&, const long, const long ) override;

  // The next two classes need to be friends to access the State_ class/member
  friend class nest::RecordablesMap< glif_psc_double_alpha >;
  friend class nest::UniversalDataLogger< glif_psc_double_alpha >;

  struct Parameters_
  {
    double G_;                           //!< membrane conductance in nS
    double E_L_;                         //!< resting potential in mV
    double th_inf_;                      //!< infinity threshold in mV
    double C_m_;                         //!< capacitance in pF
    double t_ref_;                       //!< refractory time in ms
    double V_reset_;                     //!< Membrane voltage following spike in mV
    double th_spike_add_;                //!< threshold additive constant following reset in mV
    double th_spike_decay_;              //!< spike induced threshold in 1/ms
    double voltage_reset_fraction_;      //!< voltage fraction following reset coefficient
    double voltage_reset_add_;           //!< voltage additive constant following reset in mV
    double th_voltage_index_;            //!< a 'leak-conductance' for the voltage-dependent
                                         //!< component of the threshold in 1/ms
    double th_voltage_decay_;            //!< inverse of which is the time constant of the
                                         //!< voltage-dependent component of the threshold in 1/ms
    std::vector< double > asc_init_;     //!< initial values of ASCurrents_ in pA
    std::vector< double > asc_decay_;    //!< predefined time scale in 1/ms
    std::vector< double > asc_amps_;     //!< in pA
    std::vector< double > asc_r_;        //!< coefficient
    std::vector< double > tau_syn_fast_; //!< synaptic port time constants in ms
    std::vector< double > tau_syn_slow_; //!< synaptic port time constants in ms
    std::vector< double > amp_slow_;     //!< synaptic port time constants in ms

    //! boolean flag which indicates whether the neuron has connections
    bool has_connections_;

    //! boolean flag which indicates whether the neuron has spike dependent threshold component
    bool has_theta_spike_;

    //! boolean flag which indicates whether the neuron has after spike currents
    bool has_asc_;

    //! boolean flag which indicates whether the neuron has voltage dependent threshold component
    bool has_theta_voltage_;

    size_t n_receptors_() const; //!< Returns the size of tau_syn_fast_

    Parameters_();

    void get( DictionaryDatum& ) const;
    double set( const DictionaryDatum&, Node* );
  };

  struct State_
  {
    double U_;                         //!< relative membrane potential in mV
    double threshold_;                 //!< total threshold in mV
    double threshold_spike_;           //!< spike component of threshold in mV
    double threshold_voltage_;         //!< voltage component of threshold in mV
    double I_;                         //!< external current in pA
    double I_syn_;                     //!< postsynaptic current in pA
    double I_syn_fast_;                //!< postsynaptic current in pA
    double I_syn_slow_;                //!< postsynaptic current in pA
    std::vector< double > ASCurrents_; //!< after-spike currents in pA
    double ASCurrents_sum_;            //!< in pA
    int refractory_steps_;             //!< Number of refractory steps remaining
    std::vector< double > y1_fast_;    //!< synapse current evolution state 1 in pA
    std::vector< double > y2_fast_;    //!< synapse current evolution state 2 in pA
    std::vector< double > y1_slow_;    //!< synapse current evolution state 1 in pA
    std::vector< double > y2_slow_;    //!< synapse current evolution state 2 in pA

    State_( const Parameters_& );

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };


  struct Buffers_
  {
    Buffers_( glif_psc_double_alpha& );
    Buffers_( const Buffers_&, glif_psc_double_alpha& );

    std::vector< nest::RingBuffer > spikes_; //!< Buffer incoming spikes through delay, as sum
    nest::RingBuffer currents_;              //!< Buffer incoming currents through delay,

    //! Logger for all analog data
    nest::UniversalDataLogger< glif_psc_double_alpha > logger_;
  };

  struct Variables_
  {
    int RefractoryCounts_;                             //!< counter during refractory period
    double theta_spike_decay_rate_;                    //!< threshold spike component decay rate
    double theta_spike_refractory_decay_rate_;         //!< threshold spike component decay rate during refractory
    double theta_voltage_decay_rate_inverse_;          //!< inverse of threshold voltage component decay rate
    double potential_decay_rate_;                      //!< membrane potential decay rate
    double abpara_ratio_voltage_;                      //!< ratio of parameters of voltage threshold component av/bv
    std::vector< double > asc_decay_rates_;            //!< after spike current decay rates
    std::vector< double > asc_stable_coeff_;           //!< after spike current stable coefficient
    std::vector< double > asc_refractory_decay_rates_; //!< after spike current decay rates during refractory
    double phi;                                        //!< threshold voltage component coefficient

    std::vector< double > P11_fast_; //!< synaptic current evolution parameter
    std::vector< double > P21_fast_; //!< synaptic current evolution parameter
    std::vector< double > P22_fast_; //!< synaptic current evolution parameter
    double P30_;                     //!< membrane current/voltage evolution parameter
    double P33_;                     //!< membrane voltage evolution parameter
    std::vector< double > P31_fast_; //!< synaptic/membrane current evolution parameter
    std::vector< double > P32_fast_; //!< synaptic/membrane current evolution parameter

    // slow component
    std::vector< double > P11_slow_; //!< synaptic current evolution parameter
    std::vector< double > P21_slow_; //!< synaptic current evolution parameter
    std::vector< double > P22_slow_; //!< synaptic current evolution parameter
    std::vector< double > P31_slow_; //!< synaptic/membrane current evolution parameter
    std::vector< double > P32_slow_; //!< synaptic/membrane current evolution parameter


    /** Amplitude of the synaptic current.
              This value is chosen such that a postsynaptic current with
              weight one has an amplitude of 1 pA.
    */
    std::vector< double > PSCInitialValues_;
    std::vector< double > PSCInitialValues_slow_;
  };

  double
  get_V_m_() const
  {
    return S_.U_ + P_.E_L_;
  }

  double
  get_ASCurrents_sum_() const
  {
    return S_.ASCurrents_sum_;
  }

  double
  get_I_() const
  {
    return S_.I_;
  }

  double
  get_I_syn_() const
  {
    return S_.I_syn_;
  }

  double
  get_threshold_() const
  {
    return S_.threshold_ + P_.E_L_;
  }

  double
  get_threshold_spike_() const
  {
    return S_.threshold_spike_;
  }

  double
  get_threshold_voltage_() const
  {
    return S_.threshold_voltage_;
  }

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static nest::RecordablesMap< glif_psc_double_alpha > recordablesMap_;
};


inline size_t
nest::glif_psc_double_alpha::Parameters_::n_receptors_() const
{
  return tau_syn_fast_.size();
}

inline size_t
nest::glif_psc_double_alpha::send_test_event( nest::Node& target, size_t receptor_type, nest::synindex, bool )
{
  nest::SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
nest::glif_psc_double_alpha::handles_test_event( nest::CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
nest::glif_psc_double_alpha::handles_test_event( nest::DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
glif_psc_double_alpha::get_status( DictionaryDatum& d ) const
{
  // get our own parameter and state data
  P_.get( d );
  S_.get( d, P_ );

  // get information managed by parent class
  ArchivingNode::get_status( d );

  ( *d )[ nest::names::recordables ] = recordablesMap_.get_list();
}

inline void
glif_psc_double_alpha::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

inline void
glif_psc_double_alpha::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e ); // the logger does this for us
}

template <>
void RecordablesMap< nest::glif_psc_double_alpha >::create();

} // namespace nest

#endif
