/*
 *  ht_neuron.h
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

#ifndef HT_NEURON_H
#define HT_NEURON_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C++ includes:
#include <string>
#include <vector>

// C includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

// Includes from

// Includes from sli:
#include "stringdatum.h"

namespace nest
{
/**
 * Function computing right-hand side of ODE for GSL solver.
 * @note Must be declared here so we can befriend it in class.
 * @note Must have C-linkage for passing to GSL. Internally, it is
 *       a first-class C++ function, but cannot be a member function
 *       because of the C-linkage.
 * @note No point in declaring it inline, since it is called
 *       through a function pointer.
 * @param void* Pointer to model neuron instance.
 */
extern "C" int ht_neuron_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: neuron, Hill-Tononi plasticity

Short description
+++++++++++++++++

Neuron model after Hill & Tononi (2005)

Description
+++++++++++

This model neuron implements a slightly modified version of the
neuron model described in [1]_. The most important properties are:

- Integrate-and-fire with threshold adaptive threshold.
- Repolarizing potassium current instead of hard reset.
- AMPA, NMDA, GABA_A, and GABA_B conductance-based synapses with
  beta-function (difference of exponentials) time course.
- Voltage-dependent NMDA with instantaneous or two-stage unblocking [1]_, [2]_.
- Intrinsic currents I_h, I_T, I_Na(p), and I_KNa.
- Synaptic "minis" are not implemented.

Documentation and examples:
- docs/model_details/HillTononiModels.ipynb
- pynest/examples/intrinsic_currents_spiking.py
- pynest/examples/intrinsic_currents_subthreshold.py

Parameters
++++++++++

=============== ======= =========================================================
 V_m            mV      Membrane potential
 tau_m          ms      Membrane time constant applying to all currents except
                        repolarizing K-current (see [1]_, p 1677)
 t_ref          ms      Refractory time and duration of post-spike repolarizing
                        potassium current (t_spike in [1]_)
 tau_spike      ms      Membrane time constant for post-spike repolarizing
                        potassium current
 voltage_clamp  boolean If true, clamp voltage to value at beginning of
 simulation
                        (default: false, mainly for testing)
 theta          mV      Threshold
 theta_eq       mV      Equilibrium value
 tau_theta      ms      Time constant
 g_KL           nS      Conductance for potassium leak current
 E_K            mV      Reversal potential for potassium leak currents
 g_NaL          nS      Conductance for sodium leak currents
 E_Na           mV      Reversal potential for Na leak currents
 tau_D_KNa      ms      Relaxation time constant for I_KNa
 receptor_types         Dictionary mapping synapse names to ports on neuron
 model
 recordables            List of recordable quantities
=============== ======= =========================================================

+------------------------------------------------------------+
|{E_rev,g_peak,tau_rise,tau_decay}_{AMPA,NMDA,GABA_A,GABA_B} |
+------------------------------------------------------------+
| Reversal potentials, peak conductances and time constants  |
| for synapses (tau_rise/tau_decay correspond to tau_1/tau_2 |
| in the paper)                                              |
+------------------------------------------------------------+

+------------------------+------------------------------------------------+
|V_act_NMDA, S_act_NMDA, |  Parameters for voltage dependence of NMDA-    |
|tau_Mg_{fast, slow}_NMDA|  conductance, see above                        |
+------------------------+------------------------------------------------+

============================ =================================================
instant_unblock_NMDA         Instantaneous NMDA unblocking (default: false)
{E_rev,g_peak}_{h,T,NaP,KNa} Reversal potential and peak conductance for
                             intrinsic currents
equilibrate                  If given and true, time-dependent activation
                             and inactivation state variables (h, m) of
                             intrinsic currents and NMDA channels are set
                             to their equilibrium values during this
                             SetStatus call; otherwise they retain their
                             present values.
============================ =================================================

.. note::
   Conductances are unitless in this model and currents are in mV.

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

.. [1] Hill S, Tononi G (2005). Modeling sleep and wakefulness in the
       thalamocortical system. Journal of Neurophysiology. 93:1671-1698.
       DOI: https://doi.org/10.1152/jn.00915.2004
.. [2] Vargas-Caballero M, Robinson HPC (2003). A slow fraction of Mg2+
       unblock of NMDA receptors limits their  contribution to spike generation
       in cortical pyramidal neurons. Journal of Neurophysiology 89:2778-2783.
       DOI: https://doi.org/10.1152/jn.01038.2002

See also
++++++++

ht_connection

EndUserDocs */

class ht_neuron : public Archiving_Node
{
public:
  ht_neuron();
  ht_neuron( const ht_neuron& );
  ~ht_neuron();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( SpikeEvent& e );
  void handle( CurrentEvent& e );
  void handle( DataLoggingRequest& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  /**
   * Synapse types to connect to
   * @note Excluded upper and lower bounds are defined as INF_, SUP_.
   *       Excluding port 0 avoids accidental connections.
   */
  enum SynapseTypes
  {
    INF_SPIKE_RECEPTOR = 0,
    AMPA,
    NMDA,
    GABA_A,
    GABA_B,
    SUP_SPIKE_RECEPTOR
  };

  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  double get_synapse_constant( double, double, double );

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int ht_neuron_dynamics( double, const double*, double*, void* );

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    Parameters_();

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary

    // Note: Conductances are unitless
    // Leaks
    double E_Na; // mV
    double E_K;  // mV
    double g_NaL;
    double g_KL;
    double tau_m; // ms

    // Dynamic threshold
    double theta_eq;  // mV
    double tau_theta; // ms

    // Post-spike potassium current
    double tau_spike; // ms, membrane time constant for this current
    double t_ref;     // ms, refractory time

    // Parameters for synapse of type AMPA, GABA_A, GABA_B and NMDA
    double g_peak_AMPA;
    double tau_rise_AMPA;  // ms
    double tau_decay_AMPA; // ms
    double E_rev_AMPA;     // mV

    double g_peak_NMDA;
    double tau_rise_NMDA;    // ms
    double tau_decay_NMDA;   // ms
    double E_rev_NMDA;       // mV
    double V_act_NMDA;       // mV, inactive for V << Vact, inflection of sigmoid
    double S_act_NMDA;       // mV, scale of inactivation
    double tau_Mg_slow_NMDA; // ms
    double tau_Mg_fast_NMDA; // ms
    bool instant_unblock_NMDA;

    double g_peak_GABA_A;
    double tau_rise_GABA_A;  // ms
    double tau_decay_GABA_A; // ms
    double E_rev_GABA_A;     // mV

    double g_peak_GABA_B;
    double tau_rise_GABA_B;  // ms
    double tau_decay_GABA_B; // ms
    double E_rev_GABA_B;     // mV

    // parameters for intrinsic currents
    double g_peak_NaP;
    double E_rev_NaP; // mV

    double g_peak_KNa;
    double E_rev_KNa; // mV
    double tau_D_KNa; // ms

    double g_peak_T;
    double E_rev_T; // mV

    double g_peak_h;
    double E_rev_h; // mV

    bool voltage_clamp;
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
public:
  struct State_
  {
    // y_ = [V, theta, Synapses]
    enum StateVecElems_
    {
      V_M = 0,
      THETA,
      DG_AMPA,
      G_AMPA,
      DG_NMDA_TIMECOURSE,
      G_NMDA_TIMECOURSE,
      DG_GABA_A,
      G_GABA_A,
      DG_GABA_B,
      G_GABA_B, // DO NOT INSERT ANYTHING UP TO HERE, WILL MIX UP
                // SPIKE DELIVERY
      m_fast_NMDA,
      m_slow_NMDA,
      m_Ih,
      D_IKNa,
      m_IT,
      h_IT,
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];

    /** Timer (counter) for spike-activated repolarizing potassium current.
     * Neuron is absolutely refractory during this period.
     */
    long ref_steps_;

    double I_NaP_; //!< Persistent Na current; member only to allow recording
    double I_KNa_; //!< Depol act. K current; member only to allow recording
    double I_T_;   //!< Low-thresh Ca current; member only to allow recording
    double I_h_;   //!< Pacemaker current; member only to allow recording

    State_( const ht_neuron&, const Parameters_& p );
    State_( const State_& s );
    ~State_();

    State_& operator=( const State_& s );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const ht_neuron&, Node* node );
  };

private:
  // These friend declarations must be precisely here.
  friend class RecordablesMap< ht_neuron >;
  friend class UniversalDataLogger< ht_neuron >;


  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( ht_neuron& );
    Buffers_( const Buffers_&, ht_neuron& );

    UniversalDataLogger< ht_neuron > logger_;

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spike_inputs_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // Since IntergrationStep_ is initialized with step_, and the resolution
    // cannot change after nodes have been created, it is safe to place both
    // here.
    double step_;             //!< step size in ms
    double integration_step_; //!< current integration time step, updated by GSL

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    //! size of conductance steps for arriving spikes
    std::vector< double > cond_steps_;

    //! Duration of potassium current.
    int PotassiumRefractoryCounts_;

    //! Voltage at beginning of simulation, for clamping
    double V_clamp_;
  };


  // readout functions, can use template for vector elements
  template < State_::StateVecElems_ elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }
  double
  get_I_NaP_() const
  {
    return S_.I_NaP_;
  }
  double
  get_I_KNa_() const
  {
    return S_.I_KNa_;
  }
  double
  get_I_T_() const
  {
    return S_.I_T_;
  }
  double
  get_I_h_() const
  {
    return S_.I_h_;
  }

  double get_g_NMDA_() const;

  /**
   * NMDA activation for given parameters
   * Needs to take parameter values explicitly since it is called from
   * _dynamics.
   */
  double m_NMDA_( double V, double m_eq, double m_fast, double m_slow ) const;

  /**
   * Return equilibrium value of I_h activation
   *
   * @param V Membrane potential for which to evaluate
   *        (may differ from y_[V_M] when clamping)
   */
  double m_eq_h_( double V ) const;

  /**
   * Return equilibrium value of I_T activation
   *
   * @param V Membrane potential for which to evaluate
   *        (may differ from y_[V_M] when clamping)
   */
  double m_eq_T_( double V ) const;

  /**
   * Return equilibrium value of I_T inactivation
   *
   * @param V Membrane potential for which to evaluate
   *        (may differ from y_[V_M] when clamping)
   */
  double h_eq_T_( double V ) const;

  /**
   * Return steady-state magnesium unblock ratio.
   *
   * Receives V_m as argument since it is called from ht_neuron_dyamics
   * with temporary state values.
   */
  double m_eq_NMDA_( double V ) const;

  /**
   * Steady-state "D" value for given voltage.
   */
  double D_eq_KNa_( double V ) const;

  static RecordablesMap< ht_neuron > recordablesMap_;

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};


inline port
ht_neuron::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
ht_neuron::handles_test_event( SpikeEvent&, rport receptor_type )
{
  assert( B_.spike_inputs_.size() == 4 );

  if ( not( INF_SPIKE_RECEPTOR < receptor_type && receptor_type < SUP_SPIKE_RECEPTOR ) )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
    return 0;
  }
  else
  {
    return receptor_type - 1;
  }


  /*
if (receptor_type != 0)
{
  throw UnknownReceptorType(receptor_type, get_name());
}
return 0;*/
}

inline port
ht_neuron::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
ht_neuron::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}
}

#endif // HAVE_GSL
#endif // HT_NEURON_H
