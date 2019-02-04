/*
 *  aeif_cbvg_2010.h
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

#ifndef AEIF_CBVG_2010_H
#define AEIF_CBVG_2010_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// External includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

// Includes from nestkernel:
#include "clopath_archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"


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
extern "C" int aeif_cbvg_2010_dynamics( double, const double*, double*, void* );

/** @BeginDocumentation
Name: aeif_cbvg_2010 - Exponential integrate-and-fire neuron
                        model according to Clopath et al. (2010).

Description:

aeif_cbvg_2010 is an implementation of the neuron model as it is used in [1].
It is an extension of the aeif_psc_delta model and capable of connecting to a
Clopath synapse.

Note that there are two points that are not mentioned in the paper but
present in a Matlab implementation by Claudia Clopath. The first one is the
clamping of the membrane potential to a fixed value after a spike occured to
mimik a real spike and not just the upswing. This is important since the finite
duration of the spike influences the evolution of the convolved versions
(u_bar_[plus/minus]) of the membrane potential and thus the change of the
synaptic weight. Secondly, there is a delay with which u_bar_[plus/minus] are
used to compute the change of the synaptic weight.

Parameters:

The following parameters can be set in the status dictionary.

Dynamic state variables:
V_m         double - Membrane potential in mV.
w           double - Spike-adaptation current in pA.
z           double - Spike-adaptation current in pA.
V_th        double - Adaptive spike initiation threshold in mV.

Membrane Parameters:
C_m         double - Capacity of the membrane in pF
t_ref       double - Duration of refractory period in ms.
V_reset     double - Reset value for V_m after a spike. In mV.
E_L         double - Leak reversal potential in mV.
g_L         double - Leak conductance in nS.
I_e         double - Constant external input current in pA.
tau_plus    double - Time constant of u_bar_plus.
tau_minus   double - Time constant of u_bar_minus.
tau_bar_bar double - Time constant of u_bar_bar.

Spike adaptation parameters:
a          double - Subthreshold adaptation in nS.
b          double - Spike-triggered adaptation in pA.
Delta_T    double - Slope factor in mV.
tau_w      double - Adaptation time constant in ms.
V_peak     double - Spike detection threshold in mV.
V_th_max   double - Value of V_th afer a spike in mV.
V_th_rest  double - Resting value of V_th in mV.

Clopath rule parameters:
u_bar_plus    double - Low-pass filtered Membrane potential in mV.
u_bar_minus   double - Low-pass filtered Membrane potential in mV.
u_bar_bar     double - Low-pass filtered u_bar_minus in mV.
A_LTD         double - Amplitude of depression in 1/mV.
A_LTP         double - Amplitude of facilitation in 1/mV^2.
theta_plus    double - threshold for u in mV.
theta_minus   double - threshold for u_bar_[plus/minus] in mV.
A_LTD_const   bool   - Flag that indicates whether A_LTD_ should
                       be constant (true, default) or multiplied by
                       u_bar_bar^2 / u_ref_squared (false).
delay_u_bars  double - Delay with which u_bar_[plus/minus] are processed
                       to compute the synaptic weights.
U_ref_squared double - Reference value for u_bar_bar_^2.

Other parameters:
t_clamp      double - Duration of clamping of Membrane potential after a spike
                      in ms.
V_clamp      double - Value to which the Membrane potential is clamped in mV.

Integration parameters:
gsl_error_tol double - This parameter controls the admissible error of the
                       GSL integrator. Reduce it if NEST complains about
                       numerical instabilities.

Note:

Neither the clamping nor the delayed processing of u_bar_[plus/minus] are
mentioned in [1]. However, they are part of an reference implementation
by Claudia Clopath et al. that can be found on ModelDB. The clamping is
important to mimic a spike which is otherwise not described by the aeif neuron
model.

Author: Jonas Stapmanns, David Dahmen, Jan Hahne

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

References:  [1] Clopath et al. (2010) Connectivity reflects coding:
                a model of voltage-based STDP with homeostasis.
                Nature Neuroscience 13:3, 344--352
             [2] Clopath and Gerstner (2010) Voltage and spike timing interact
                in STDP – a unified model. Front. Synaptic Neurosci. 2:25
                doi: 10.3389/fnsyn.2010.00025

SeeAlso: aeif_psc_delta, clopath_synapse, hh_psc_alpha_clopath
*/
class aeif_cbvg_2010 : public Clopath_Archiving_Node
{

public:
  aeif_cbvg_2010();
  aeif_cbvg_2010( const aeif_cbvg_2010& );
  ~aeif_cbvg_2010();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();
  void update( const Time&, const long, const long );

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int aeif_cbvg_2010_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< aeif_cbvg_2010 >;
  friend class UniversalDataLogger< aeif_cbvg_2010 >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {
    double V_peak_;  //!< Spike detection threshold in mV
    double V_reset_; //!< Reset Potential in mV
    double t_ref_;   //!< Refractory period in ms

    double g_L;       //!< Leak Conductance in nS
    double C_m;       //!< Membrane Capacitance in pF
    double E_L;       //!< Leak reversal Potential (aka resting potential) in mV
    double Delta_T;   //!< Slope faktor in ms
    double tau_w;     //!< adaptation time-constant in ms
    double tau_z;     //!< adaptation time-constant in ms
    double tau_V_th;  //!< adaptive threshold time-constant in ms
    double V_th_max;  //!< value of V_th afer a spike in mV
    double V_th_rest; //!< resting value of V_th in mV
    double tau_plus;  //!< time constant of u_bar_plus in ms
    double tau_minus; //!< time constant of u_bar_minus in ms
    double tau_bar_bar; //!< time constant of u_bar_bar in ms
    double a;           //!< Subthreshold adaptation in nS.
    double b;           //!< Spike-triggered adaptation in pA
    double I_sp;
    double t_ref; //!< Refractory period in ms.
    double I_e;   //!< Intrinsic current in pA.

    double gsl_error_tol; //!< error bound for GSL integrator

    double t_clamp_; //!< The membrane potential is clamped to V_clamp (in mV)
    double V_clamp_; //!< for the duration of t_clamp (in ms) after each spike.

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor and assignment operator required because
   *       of C-style array.
   */
  struct State_
  {
    /**
     * Enumeration identifying elements in state array State_::y_.
     * The state vector must be passed to GSL as a C array. This enum
     * identifies the elements of the vector. It must be public to be
     * accessible from the iteration function.
     */
    enum StateVecElems
    {
      V_M = 0,
      W,           // 1
      Z,           // 2
      V_TH,        // 3
      U_BAR_PLUS,  // 4
      U_BAR_MINUS, // 5
      U_BAR_BAR,   // 6
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    unsigned int r_;       //!< number of refractory steps remaining
    unsigned int clamp_r_; //!< number of clamp steps remaining

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( aeif_cbvg_2010& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, aeif_cbvg_2010& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< aeif_cbvg_2010 > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing the GSL system

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
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
    double I_stim_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /**
     * Threshold detection for spike events: P.V_peak if Delta_T > 0.,
     * S_.y_[ State_::V_TH ] if Delta_T == 0.
     */
    double V_peak_;

    unsigned int refractory_counts_;
    unsigned int clamp_counts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< aeif_cbvg_2010 > recordablesMap_;
};

inline port
aeif_cbvg_2010::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
aeif_cbvg_2010::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_cbvg_2010::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_cbvg_2010::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
aeif_cbvg_2010::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Clopath_Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
aeif_cbvg_2010::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, ptmp );   // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Clopath_Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // HAVE_GSL
#endif // AEIF_CBVG_2010_H
