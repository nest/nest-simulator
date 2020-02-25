/*
 *  aeif_psc_delta_clopath.h
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

#ifndef AEIF_PSC_DELTA_CLOPATH_H
#define AEIF_PSC_DELTA_CLOPATH_H

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
extern "C" int aeif_psc_delta_clopath_dynamics( double, const double*, double*, void* );

/** @BeginDocumentation
@ingroup Neurons
@ingroup iaf
@ingroup clopath_n
@ingroup aeif
@ingroup psc

Name: aeif_psc_delta_clopath - Exponential integrate-and-fire neuron
model according to Clopath et al. (2010).

Description:

aeif_psc_delta_clopath is an implementation of the neuron model as it is used
in [1]. It is an extension of the aeif_psc_delta model and capable of
connecting to a Clopath synapse.

Note that there are two points that are not mentioned in the paper but
present in a MATLAB implementation by Claudia Clopath [3]. The first one is the
clamping of the membrane potential to a fixed value after a spike occured to
mimik a real spike and not just the upswing. This is important since the finite
duration of the spike influences the evolution of the convolved versions
(u_bar_[plus/minus]) of the membrane potential and thus the change of the
synaptic weight. Secondly, there is a delay with which u_bar_[plus/minus] are
used to compute the change of the synaptic weight.

Parameters:

The following parameters can be set in the status dictionary.

\verbatim embed:rst

=========== ======  ===================================================
**Dynamic state variables**
-----------------------------------------------------------------------
V_m         mV      Membrane potential
w           pA      Spike-adaptation current
z           pA      Spike-adaptation current
V_th        mV      Adaptive spike initiation threshold
u_bar_plus  mV      Low-pass filtered Membrane potential
u_bar_minus mV      Low-pass filtered Membrane potential
u_bar_bar   mV      Low-pass filtered u_bar_minus
=========== ======  ===================================================

============ ======  =================================================
**Membrane Parameters**
----------------------------------------------------------------------
 C_m         pF      Capacity of the membrane
 t_ref       ms      Duration of refractory period
 V_reset     mV      Reset value for V_m after a spike
 E_L         mV      Leak reversal potential
 g_L         nS      Leak conductance
 I_e         pA      Constant external input current
 tau_plus    ms      Time constant of u_bar_plus
 tau_minus   ms      Time constant of u_bar_minus
 tau_bar_bar ms      Time constant of u_bar_bar
============ ======  =================================================


========== ======  ===================================================
**Spike adaptation parameters**
----------------------------------------------------------------------
a          nS      Subthreshold adaptation
b          pA      Spike-triggered adaptation
Delta_T    mV      Slope factor
tau_w      ms      Adaptation time constant
V_peak     mV      Spike detection threshold
V_th_max   mV      Value of V_th afer a spike
V_th_rest  mV      Resting value of V_th
========== ======  ===================================================

============= ======= =======================================================
**Clopath rule parameters**
-----------------------------------------------------------------------------
A_LTD         1/mV    Amplitude of depression
A_LTP         1/mV^2  Amplitude of facilitation
theta_plus    mV      Threshold for u
theta_minus   mV      Threshold for u_bar_[plus/minus]
A_LTD_const   boolean Flag that indicates whether `A_LTD_` should
                      be constant (true, default) or multiplied by
                      u_bar_bar^2 / u_ref_squared (false).
delay_u_bars  real    Delay with which u_bar_[plus/minus] are processed
                      to compute the synaptic weights.
U_ref_squared real    Reference value for u_bar_bar_^2.
============= ======= =======================================================


=======  ====== =============================================================
**Other parameters**
-----------------------------------------------------------------------------
t_clamp  ms     Duration of clamping of Membrane potential after a spike
V_clamp  mV     Value to which the Membrane potential is clamped
=======  ====== =============================================================


============= ======= =========================================================
**Integration parameters**
-------------------------------------------------------------------------------
gsl_error_tol real    This parameter controls the admissible error of the
                      GSL integrator. Reduce it if NEST complains about
                      numerical instabilities.
============= ======= =========================================================
\endverbatim

Note:

Neither the clamping nor the delayed processing of u_bar_[plus/minus] are
mentioned in [1]. However, they are part of an reference implementation
by Claudia Clopath et al. that can be found on ModelDB [3]. The clamping is
important to mimic a spike which is otherwise not described by the aeif neuron
model.

Author: Jonas Stapmanns, David Dahmen, Jan Hahne

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

References:
\verbatim embed:rst
.. [1] Clopath et al. (2010). Connectivity reflects coding: a model of
       voltage-based STDP with homeostasis. Nature Neuroscience 13(3):344-352.
       DOI: https://doi.org/10.1038/nn.2479
.. [2] Clopath and Gerstner (2010). Voltage and spike timing interact
       in STDP – a unified model. Frontiers in Synaptic Neuroscience. 2:25
       DOI: https://doi.org/10.3389/fnsyn.2010.00025
.. [3] Voltage-based STDP synapse (Clopath et al. 2010) on ModelDB
       https://senselab.med.yale.edu/ModelDB/showmodel.cshtml?model=144566&file=%2f
       modeldb_package%2fVoTriCode%2faEIF.m
\endverbatim
SeeAlso: aeif_psc_delta, clopath_synapse, hh_psc_alpha_clopath
*/
class aeif_psc_delta_clopath : public Clopath_Archiving_Node
{

public:
  aeif_psc_delta_clopath();
  aeif_psc_delta_clopath( const aeif_psc_delta_clopath& );
  ~aeif_psc_delta_clopath();

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
  friend int aeif_psc_delta_clopath_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< aeif_psc_delta_clopath >;
  friend class UniversalDataLogger< aeif_psc_delta_clopath >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {
    double V_peak_;  //!< Spike detection threshold in mV
    double V_reset_; //!< Reset Potential in mV
    double t_ref_;   //!< Refractory period in ms

    double g_L;         //!< Leak Conductance in nS
    double C_m;         //!< Membrane Capacitance in pF
    double E_L;         //!< Leak reversal Potential (aka resting potential) in mV
    double Delta_T;     //!< Slope faktor in ms
    double tau_w;       //!< adaptation time-constant in ms
    double tau_z;       //!< adaptation time-constant in ms
    double tau_V_th;    //!< adaptive threshold time-constant in ms
    double V_th_max;    //!< value of V_th afer a spike in mV
    double V_th_rest;   //!< resting value of V_th in mV
    double tau_plus;    //!< time constant of u_bar_plus in ms
    double tau_minus;   //!< time constant of u_bar_minus in ms
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

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
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
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( aeif_psc_delta_clopath& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, aeif_psc_delta_clopath& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< aeif_psc_delta_clopath > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing the GSL system

    // Since IntergrationStep_ is initialized with step_, and the resolution
    // cannot change after nodes have been created, it is safe to place both
    // here.
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
  static RecordablesMap< aeif_psc_delta_clopath > recordablesMap_;
};

inline port
aeif_psc_delta_clopath::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
aeif_psc_delta_clopath::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_psc_delta_clopath::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_psc_delta_clopath::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
aeif_psc_delta_clopath::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Clopath_Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
aeif_psc_delta_clopath::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

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
#endif // AEIF_PSC_DELTA_CLOPATH_H
