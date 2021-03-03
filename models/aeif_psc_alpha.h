/*
 *  aeif_psc_alpha.h
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

#ifndef AEIF_PSC_ALPHA_H
#define AEIF_PSC_ALPHA_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// External includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

// Includes from nestkernel:
#include "archiving_node.h"
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
extern "C" int aeif_psc_alpha_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: neuron, adaptive threshold, integrate-and-fire, current-based

Short description
+++++++++++++++++

Current-based exponential integrate-and-fire neuron model

Description
+++++++++++

aeif_psc_alpha is the adaptive exponential integrate and fire neuron according
to Brette and Gerstner (2005).
Synaptic currents are modelled as alpha-functions.

This implementation uses the embedded 4th order Runge-Kutta-Fehlberg solver with
adaptive step size to integrate the differential equation.

The membrane potential is given by the following differential equation:

.. math::

 C dV/dt= -g_L(V-E_L)+g_L*\Delta_T*\exp((V-V_T)/\Delta_T)-g_e(t)(V-E_e) \\
                                                     -g_i(t)(V-E_i)-w +I_e

and

.. math::

 \tau_w * dw/dt= a(V-E_L) -W

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======== ======= =======================================
**Dynamic state variables:**
--------------------------------------------------------
 V_m     mV      Membrane potential
 I_ex    pA      Excitatory synaptic current
 dI_ex   pA/ms   First derivative of I_ex
 I_in    pA      Inhibitory synaptic current
 dI_in   pA/ms   First derivative of I_in
 w       pA      Spike-adaptation current
 g       pa      Spike-adaptation current
======== ======= =======================================

======== ======= =======================================
**Membrane Parameters**
--------------------------------------------------------
 C_m     pF      Capacity of the membrane
 t_ref   ms      Duration of refractory period
 V_reset mV      Reset value for V_m after a spike
 E_L     mV      Leak reversal potential
 g_L     nS      Leak conductance
 I_e     pA      Constant external input current
======== ======= =======================================

======== ======= ==================================
**Spike adaptation parameters**
---------------------------------------------------
 a       ns      Subthreshold adaptation
 b       pA      Spike-triggered adaptation
 Delta_T mV      Slope factor
 tau_w   ms      Adaptation time constant
 V_th    mV      Spike initiation threshold
 V_peak  mV      Spike detection threshold
======== ======= ==================================

=========== ======= ===========================================================
**Synaptic parameters**
-------------------------------------------------------------------------------
 tau_syn_ex ms      Rise time of excitatory synaptic conductance (alpha
                    function)
 tau_syn_in ms      Rise time of the inhibitory synaptic conductance
                    (alpha function)
=========== ======= ===========================================================

============= ======= =========================================================
**Integration parameters**
-------------------------------------------------------------------------------
gsl_error_tol real    This parameter controls the admissible error of the
                      GSL integrator. Reduce it if NEST complains about
                      numerical instabilities
============= ======= =========================================================

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

.. [1] Brette R and Gerstner W (2005). Adaptive Exponential
       Integrate-and-Fire Model as an Effective Description of Neuronal
       Activity. J Neurophysiol 94:3637-3642.
       DOI: https://doi.org/10.1152/jn.00686.2005

See also
++++++++

iaf_psc_alpha, aeif_cond_exp

EndUserDocs */

class aeif_psc_alpha : public ArchivingNode
{

public:
  aeif_psc_alpha();
  aeif_psc_alpha( const aeif_psc_alpha& );
  ~aeif_psc_alpha();

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
  void update( Time const&, const long, const long );

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int aeif_psc_alpha_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< aeif_psc_alpha >;
  friend class UniversalDataLogger< aeif_psc_alpha >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {
    double V_peak_;  //!< Spike detection threshold in mV
    double V_reset_; //!< Reset Potential in mV
    double t_ref_;   //!< Refractory period in ms

    double g_L;        //!< Leak Conductance in nS
    double C_m;        //!< Membrane Capacitance in pF
    double E_L;        //!< Leak reversal Potential (aka resting potential) in mV
    double Delta_T;    //!< Slope factor in ms
    double tau_w;      //!< Adaptation time-constant in ms
    double a;          //!< Subthreshold adaptation in nS
    double b;          //!< Spike-triggered adaptation in pA
    double V_th;       //!< Spike threshold in mV
    double t_ref;      //!< Refractory period in ms
    double tau_syn_ex; //!< Excitatory synaptic rise time
    double tau_syn_in; //!< Excitatory synaptic rise time
    double I_e;        //!< Intrinsic current in pA

    double gsl_error_tol; //!< Error bound for GSL integrator

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
      DI_EXC, // 1
      I_EXC,  // 2
      DI_INH, // 3
      I_INH,  // 4
      W,      // 5
      STATE_VEC_SIZE
    };

    double y_[ STATE_VEC_SIZE ]; //!< neuron state, must be C-array for
                                 //!< GSL solver
    unsigned int r_;             //!< number of refractory steps remaining

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
    Buffers_( aeif_psc_alpha& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, aeif_psc_alpha& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< aeif_psc_alpha > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
    RingBuffer spike_inh_;
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
    /** initial value to normalise excitatory synaptic current */
    double i0_ex_;

    /** initial value to normalise inhibitory synaptic current */
    double i0_in_;

    /**
     * Threshold detection for spike events: P.V_peak if Delta_T > 0.,
     * P.V_th if Delta_T == 0.
     */
    double V_peak;

    unsigned int refractory_counts_;
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
  static RecordablesMap< aeif_psc_alpha > recordablesMap_;
};

inline port
aeif_psc_alpha::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
aeif_psc_alpha::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_psc_alpha::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_psc_alpha::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
aeif_psc_alpha::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
aeif_psc_alpha::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // HAVE_GSL
#endif // AEIF_PSC_ALPHA_H
