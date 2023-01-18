/*
 *  glif_cond.h
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

#ifndef GLIF_COND_H
#define GLIF_COND_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

#include "dictdatum.h"

/* BeginUserDocs: integrate-and-fire, conductance-based

Short description
+++++++++++++++++

Conductance-based generalized leaky integrate and fire (GLIF) model (from the Allen Institute)

Description
+++++++++++

``glif_cond`` provides five generalized leaky integrate
and fire (GLIF) models [1]_ with conductance-based synapses.
Incoming spike events induce a postsynaptic change of conductance modeled
by an alpha function [2]_. The alpha function is normalized such that an event
of weight 1.0 results in a peak conductance change of 1 nS at :math:`t = tau_syn`. On
the postsynaptic side, there can be arbitrarily many synaptic time constants.
This can be reached by specifying separate receptor ports, each for a different
time constant. The port number has to match the respective ``receptor_type`` in
the connectors.

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

============= ======= ======= ======
**Parameter settings**
------------------------------------
GLIF Model 1   False   False   False
GLIF Model 2   True    False   False
GLIF Model 3   False   True    False
GLIF Model 4   True    True    False
GLIF Model 5   True    True    True
============= ======= ======= ======

Typical parameter setting of different levels of GLIF models for different cells
can be found and downloaded in the `Allen Cell Type Database
<https://celltypes.brain-map.org>`_. For example, the default parameter setting of this
glif_cond neuron model was from the parameter values of GLIF Model 5 of Cell
490626718, which can be retrieved from the `Allen Brain Atlas
<https://celltypes.brain-map.org/mouse/experiment/electrophysiology/
490626718>`_, with units being converted from SI units (i.e., V, S (1/Ohm),
F, s, A) to NEST used units (i.e., mV, nS (1/GOhm), pF, ms, pA) and values
being rounded to appropriate digits for simplification.

For models with spike dependent threshold (i.e., GLIF 2, GLIF 4 and GLIF 5),
parameter setting of voltage_reset_fraction and voltage_reset_add may lead to the
situation that voltage is bigger than threshold after reset. In this case, the neuron
will continue to spike until the end of the simulation regardless the stimulated inputs.
We recommend the setting of the parameters of these three models to follow the
condition of :math:`(E_L + voltage_reset_fraction * ( V_th - E_L ) + voltage_reset_add)
< (V_th + th_spike_add)`.

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
tau_syn                    double vector  Rise time constants of the synaptic
                                          alpha function in ms
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

gif_psc_exp_multisynapse, gif_cond_exp, gif_cond_exp_multisynapse, gif_pop_psc_exp

EndUserDocs */

namespace nest
{

extern "C" int glif_cond_dynamics( double, const double*, double*, void* );

class glif_cond : public nest::ArchivingNode
{
public:
  glif_cond();

  glif_cond( const glif_cond& );

  ~glif_cond() override;

  using nest::Node::handle;
  using nest::Node::handles_test_event;

  nest::port send_test_event( nest::Node&, nest::port, nest::synindex, bool ) override;

  void handle( nest::SpikeEvent& ) override;
  void handle( nest::CurrentEvent& ) override;
  void handle( nest::DataLoggingRequest& ) override;

  nest::port handles_test_event( nest::SpikeEvent&, nest::port ) override;
  nest::port handles_test_event( nest::CurrentEvent&, nest::port ) override;
  nest::port handles_test_event( nest::DataLoggingRequest&, nest::port ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  //! Reset internal buffers of neuron.
  void init_buffers_() override;

  //! Initialize auxiliary quantities, leave parameters and state untouched.
  void pre_run_hook() override;

  //! Take neuron through given time interval
  void update( nest::Time const&, const long, const long ) override;

  // make dynamics function quasi-member
  friend int glif_cond_dynamics( double, const double*, double*, void* );

  // The next three classes need to be friends to access the State_ class/member
  friend class DynamicRecordablesMap< glif_cond >;
  friend class DynamicUniversalDataLogger< glif_cond >;
  friend class DataAccessFunctor< glif_cond >;

  struct Parameters_
  {
    double G_;                        //!< membrane conductance in nS
    double E_L_;                      //!< resting potential in mV
    double th_inf_;                   //!< infinity threshold in mV
    double C_m_;                      //!< capacitance in pF
    double t_ref_;                    //!< refractory time in ms
    double V_reset_;                  //!< Membrane voltage following spike in mV
    double th_spike_add_;             //!< threshold additive constant following reset in mV
    double th_spike_decay_;           //!< spike induced threshold in 1/ms
    double voltage_reset_fraction_;   //!< voltage fraction following reset coefficient
    double voltage_reset_add_;        //!< voltage additive constant following reset in mV
    double th_voltage_index_;         //!< a 'leak-conductance' for the voltage-dependent
                                      //!< component of the threshold in 1/ms
    double th_voltage_decay_;         //!< inverse of which is the time constant of the
                                      //!< voltage-dependent component of the threshold in 1/ms
    std::vector< double > asc_init_;  //!< initial values of ASCurrents_ in pA
    std::vector< double > asc_decay_; //!< predefined time scale in 1/ms
    std::vector< double > asc_amps_;  //!< in pA
    std::vector< double > asc_r_;     //!< coefficient
    std::vector< double > tau_syn_;   //!< synaptic port time constants in ms
    std::vector< double > E_rev_;     //!< reversal potential in mV

    //! boolean flag which indicates whether the neuron has connections
    bool has_connections_;

    //! boolean flag which indicates whether the neuron has spike dependent threshold component
    bool has_theta_spike_;

    //! boolean flag which indicates whether the neuron has after spike currents
    bool has_asc_;

    //! boolean flag which indicates whether the neuron has voltage dependent threshold component
    bool has_theta_voltage_;

    size_t n_receptors_() const; //!< Returns the size of tau_syn_

    Parameters_();

    void get( DictionaryDatum& ) const;
    double set( const DictionaryDatum&, Node* );
  };


  struct State_
  {

    double threshold_;                 //!< voltage threshold in mV
    double threshold_spike_;           //!< spike component of threshold in mV
    double threshold_voltage_;         //!< voltage component of threshold in mV
    std::vector< double > ASCurrents_; //!< after-spike currents in pA
    double ASCurrents_sum_;            //!< in pA
    int refractory_steps_;             //!< Number of refractory steps remaining

    //! Symbolic indices to the elements of the state vector y and recordables
    //! y only includes state of V_M and DG_SYN, G_SYN
    //! repeat DG_SYN, G_SYN if more receptors
    //! Recordable indices, I, ASC, TH, Th_SPK, TH_VLT, are for
    //! injection currents, sum of ASC currents, thresholds, spike component of
    //! thresholds, voltage component of thresholds recordings retrieve.
    enum StateVecElems
    {
      V_M = 0,
      I,
      ASC_SUM,
      TH,
      TH_SPK,
      TH_VLT,
      DG_SYN,
      G_SYN,
      STATE_VECTOR_MIN_SIZE
    };

    static const size_t NUMBER_OF_FIXED_STATES_ELEMENTS = 1;         // V_M
    static const size_t NUMBER_OF_RECORDABLES_ELEMENTS = DG_SYN - 1; // I, ASC, TH, Th_SPK, TH_VLT
    static const size_t NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR = 2;  // DG_SYN, G_SYN

    std::vector< double > y_; //!< neuron state

    State_( const Parameters_& );

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };


  struct Buffers_
  {
    Buffers_( glif_cond& );
    Buffers_( const Buffers_&, glif_cond& );

    std::vector< nest::RingBuffer > spikes_; //!< Buffer incoming spikes through delay, as sum
    nest::RingBuffer currents_;              //!< Buffer incoming currents through delay,

    //! Logger for all analog data
    DynamicUniversalDataLogger< glif_cond > logger_;

    /* GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // IntegrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
    double step_;            //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_;
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

    /** Amplitude of the synaptic conductance.
        This value is chosen such that an event of weight 1.0 results in a peak
        conductance of 1 nS at t = tau_syn.
    */
    std::vector< double > CondInitialValues_;
  };

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  // Mapping of recordables names to access functions
  DynamicRecordablesMap< glif_cond > recordablesMap_;

  // Data Access Functor getter
  DataAccessFunctor< glif_cond > get_data_access_functor( size_t elem );
  inline double
  get_state_element( size_t elem )
  {
    if ( elem == nest::glif_cond::State_::V_M )
    {
      return S_.y_[ elem ] + P_.E_L_;
    }
    else if ( elem == nest::glif_cond::State_::I )
    {
      return B_.I_;
    }
    else if ( elem == nest::glif_cond::State_::ASC_SUM )
    {
      return S_.ASCurrents_sum_;
    }
    else if ( elem == nest::glif_cond::State_::TH )
    {
      return S_.threshold_ + P_.E_L_;
    }
    else if ( elem == nest::glif_cond::State_::TH_SPK )
    {
      return S_.threshold_spike_;
    }
    else if ( elem == nest::glif_cond::State_::TH_VLT )
    {
      return S_.threshold_voltage_;
    }
    else
    {
      return S_.y_[ elem - nest::glif_cond::State_::NUMBER_OF_RECORDABLES_ELEMENTS ];
    }
  };

  // Utility function that inserts the synaptic conductances to the
  // recordables map

  Name get_g_receptor_name( size_t receptor );
  void insert_conductance_recordables( size_t first = 0 );
};


inline size_t
nest::glif_cond::Parameters_::n_receptors_() const
{
  return tau_syn_.size();
}


inline nest::port
nest::glif_cond::send_test_event( nest::Node& target, nest::port receptor_type, nest::synindex, bool )
{
  nest::SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline nest::port
nest::glif_cond::handles_test_event( nest::CurrentEvent&, nest::port receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline nest::port
nest::glif_cond::handles_test_event( nest::DataLoggingRequest& dlr, nest::port receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
glif_cond::get_status( DictionaryDatum& d ) const
{
  // get our own parameter and state data
  P_.get( d );
  S_.get( d, P_ );

  // get information managed by parent class
  ArchivingNode::get_status( d );

  ( *d )[ nest::names::recordables ] = recordablesMap_.get_list();
}

inline void
glif_cond::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

  ArchivingNode::set_status( d );

  /*
   * Here is where we must update the recordablesMap_ if new receptors
   * are added!
   */
  if ( ptmp.n_receptors_() > P_.n_receptors_() ) // Number of receptors increased
  {
    for ( size_t receptor = P_.n_receptors_(); receptor < ptmp.n_receptors_(); ++receptor )
    {
      size_t elem = glif_cond::State_::G_SYN + receptor * glif_cond::State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR;
      recordablesMap_.insert( get_g_receptor_name( receptor ), get_data_access_functor( elem ) );
    }
  }
  else if ( ptmp.n_receptors_() < P_.n_receptors_() )
  { // Number of receptors decreased
    for ( size_t receptor = ptmp.n_receptors_(); receptor < P_.n_receptors_(); ++receptor )
    {
      recordablesMap_.erase( get_g_receptor_name( receptor ) );
    }
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace nest

#endif // HAVE_GSL
#endif
