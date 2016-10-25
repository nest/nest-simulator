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

// Includes from sli:
#include "stringdatum.h"

/* BeginDocumentation
   Name: ht_neuron - Neuron model after Hill & Tononi (2005).

   Description:
   This model neuron implements a slightly modified version of the
   neuron model described in [1]. The most important properties are:

   - Integrate-and-fire with threshold that is increased on spiking
     and decays back to an equilibrium value.
   - No hard reset, but repolarizing potassium current.
   - AMPA, NMDA, GABA_A, and GABA_B conductance-based synapses with
     beta-function (difference of two exponentials) time course.
   - Voltage-dependent NMDA with instantaneous blocking and two-stage
     unblocking as described in [1].
   - Intrinsic currents I_h (pacemaker), I_T (low-threshold calcium),
     I_Na(p) (persistent sodium), and I_KNa (depolarization-activated
     potassium).
   - Several apparent typographical errors in the descriptions of
     the intrinsic currents were fixed, hopefully in a meaningful
     way.

   NMDA conductances is modeled as follows

     g_NMDA(t, V) = m(V, t) g(t)

   where

     g(t) = g_peak ( e^(-t/tau_1) - e^(-t/tau_2) )
               / ( e^(-t_peak/tau_1) - e^(-t_peak/tau_2) )

     t_peak = tau_2 tau_1 / ( tau_2 - tau_1 ) ln( tau_2 / tau_1 )

     m(V, t) = a(V) m_fast*(V, t) + ( 1 - a(V) ) m_slow*(V, t)
     a(V)    = 0.51 - 0.0028 V

   represents the voltage-dependence with components for fast and slow
   unblocking. For a given V, the steady-state value is given by a
   sigmoidal function

      m_ss(V) = 1 / ( 1 + exp( -NMDA_Sact ( V - NMDA_Vact ) ) )

   Instantaneous blocking means that

      m_X*(V, t) = min(m_ss(V), m_X(V, t))  for X: slow, fast

   while unblocking occurs at two different speeds according to

      dm_X / dt = ( m_ss(V) - m_X ) / NMDA_tau_Mg_X  for X: slow, fast


   I am grateful to thank Sean Hill for giving me access to his Synthesis
   simulator source code.

   Examples:
   - pynest/examples/intrinsic_currents_spiking.py
   - pynest/examples/intrinsic_currents_subthreshold.py

   Note:
   THIS MODEL NEURON HAS NOT BEEN TESTED EXTENSIVELY!

   Parameters:
   V_m            -  membrane potential
   spike_duration - duration of re-polarizing potassium current
   tau_m          - membrane time constant applying to all currents but
                    repolarizing K-current (see [1, p 1677])
   tau_spike      - membrane time constant applying to repolarizing K-current
   theta, theta_eq, tau_theta - Threshold, equilibrium value, time constant
   g_KL, E_K, g_NaL, E_Na     - conductances and reversal potentials for K and
                                Na leak currents
   {AMPA,NMDA,GABA_A,GABA_B}_{E_rev,g_peak,tau_1,tau_2}
                                - reversal potentials, peak conductances and
                                  time constants for synapses (tau_1: rise time,
                                  tau_2: decay time, tau_1 < tau_2)
   NMDA_Sact, NMDA_Vact, NMDA_tau_Mg_{fast, slow}
                                - Parameters for voltage dependence of NMDA-
                                  conductance, see above
        - Time constants for NMDA Mg unblocking,
                                  see [1, p 1678]
   {h,T,NaP,KNa}_{E_rev,g_peak} - reversal potential and peak conductance for
                                  intrinsic currents
   receptor_types               - dictionary mapping synapse names to ports on
                                  neuron model
   recordables                  - list of recordable quantities.

   Author: Hans Ekkehard Plesser

   Sends: SpikeEvent

   Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

   FirstVersion: October 2009; full NMDA model September 2016

   References:
   [1] S Hill and G Tononi (2005). J Neurophysiol 93:1671-1698.

   SeeAlso: ht_synapse
*/

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
    // Leaks
    double E_Na;  // 30 mV
    double E_K;   // -90 mV
    double g_NaL; // 0.2
    double g_KL;  // 1.0 - 1.85
    double tau_m; // ms

    // Dynamic threshold
    double theta_eq;  // mV
    double tau_theta; // ms

    // Spike potassium current
    double tau_spike; // ms
    double t_spike;   // ms

    Parameters_();

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary

    // Parameters for synapse of type AMPA, GABA_A, GABA_B and NMDA
    double AMPA_g_peak;
    double AMPA_tau_1; // ms
    double AMPA_tau_2; // ms
    double AMPA_E_rev; // mV

    double NMDA_g_peak;
    double NMDA_tau_1; // ms
    double NMDA_tau_2; // ms
    double NMDA_E_rev; // mV
    double NMDA_Vact;  //!< mV, inactive for V << Vact, inflection of sigmoid
    double NMDA_Sact;  //!< mV, scale of inactivation
    double NMDA_tau_Mg_slow; // ms
    double NMDA_tau_Mg_fast; // ms

    double GABA_A_g_peak;
    double GABA_A_tau_1; // ms
    double GABA_A_tau_2; // ms
    double GABA_A_E_rev; // mV

    double GABA_B_g_peak;
    double GABA_B_tau_1; // ms
    double GABA_B_tau_2; // ms
    double GABA_B_E_rev; // mV

    // parameters for intrinsic currents
    double NaP_g_peak;
    double NaP_E_rev; // mV

    double KNa_g_peak;
    double KNa_E_rev; // mV

    double T_g_peak;
    double T_E_rev; // mV

    double h_g_peak;
    double h_E_rev; // mV
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
      VM = 0,
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
      Mg_slow,
      Mg_fast,
      IKNa_D,
      IT_m,
      IT_h,
      Ih_m,
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];

    //! Timer (counter) for spike-activated repolarizing potassium current.
    int r_spike_;

    bool g_spike_; //!< active / not active

    double I_NaP_; //!< Persistent Na current; member only to allow recording
    double I_KNa_; //!< Depol act. K current; member only to allow recording
    double I_T_;   //!< Low-thresh Ca current; member only to allow recording
    double I_h_;   //!< Pacemaker current; member only to allow recording

    State_();
    State_( const Parameters_& p );
    State_( const State_& s );
    ~State_();

    State_& operator=( const State_& s );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
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

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
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
  };


  // readout functions, can use template for vector elements
  template < State_::StateVecElems_ elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }
  double
  get_r_spike_() const
  {
    return S_.r_spike_;
  }
  double
  get_g_spike_() const
  {
    return S_.g_spike_;
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

  double
  get_g_NMDA_() const
  {
    const double A1 = 0.51 - 0.0028 * S_.y_[ State_::VM ];
    const double A2 = 1 - A1;
    return S_.y_[ State_::G_NMDA_TIMECOURSE ]
      * ( A1 * S_.y_[ State_::Mg_fast ] + A2 * S_.y_[ State_::Mg_slow ] );
  }

  /**
   * Return steady-state magnesium unblock ratio.
   *
   * Receives V_m as argument since it is called from ht_neuron_dyamics
   * with temporary state values.
   */
  double
  Mg_steady_state_( double V ) const
  {
    return 1.0 / ( 1.0 + std::exp( -P_.NMDA_Sact * ( V - P_.NMDA_Vact ) ) );
  }

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

  if ( !( INF_SPIKE_RECEPTOR < receptor_type
         && receptor_type < SUP_SPIKE_RECEPTOR ) )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
    return 0;
  }
  else
    return receptor_type - 1;


  /*
if (receptor_type != 0)
throw UnknownReceptorType(receptor_type, get_name());
return 0;*/
}

inline port
ht_neuron::handles_test_event( CurrentEvent&, rport receptor_type )
{

  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline port
ht_neuron::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}
}

#endif // HAVE_GSL
#endif // HT_NEURON_H
