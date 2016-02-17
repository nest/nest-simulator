/*
 *  aeif_psc_exp_ps.h
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

#ifndef AEIF_PSC_EXP_PS_H
#define AEIF_PSC_EXP_PS_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL_1_11

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

// Include from precise
#include "slice_ring_buffer_new.h"

namespace nest
{

/* BeginDocumentation
Name: gp_aeif_cond_alpha - Current-based exponential integrate-and-fire
  neuron model according to Brette and Gerstner (2005), implementing precise
  spikes, as well as a linear interpolation to find the "exact" time where the
  threshold was crossed, i.e. the spiking time.

Description:
aeif_cond_alpha is the adaptive exponential integrate and fire neuron according
to Brette and Gerstner (2005) and synaptic currents are modelled as exponential
functions. This model implements precise spikes and a linear interpolation to
find spike times more precisely.

This implementation uses the embedded 4th order Runge-Kutta-Fehlberg solver
with adaptive stepsize to integrate the differential equation.

The membrane potential is given by the following differential equation:

C dV/dt = -g_L*(V-E_L) + g_L*Delta_T*exp((V-V_T)/Delta_T) + I_ex(t) - I_in(t)
          - w + I_e

and

tau_w * dw/dt = a*(V-E_L) - w

Parameters:
The following parameters can be set in the status dictionary.

Dynamic state variables:
  V_m        double - Membrane potential in mV
  I_ex       double - Excitatory synaptic current in pA.
  I_in       double - Inhibitory synaptic current in pA.
  w          double - Spike-adaptation current in pA.

Membrane Parameters:
  C_m        double - Capacity of the membrane in pF
  t_ref      double - Duration of refractory period in ms.
  V_reset    double - Reset value for V_m after a spike. In mV.
  E_L        double - Leak reversal potential in mV.
  g_L        double - Leak conductance in nS.
  I_e        double - Constant external input current in pA.

Spike adaptation parameters:
  a          double - Subthreshold adaptation in nS.
  b          double - Spike-triggered adaptation in pA.
  Delta_T    double - Slope factor in mV
  tau_w      double - Adaptation time constant in ms
  V_th       double - Spike initiation threshold in mV
  V_peak     double - Spike detection threshold in mV.

Synaptic parameters
  tau_syn_ex double - Characteristic decrease time of excitatory synaptic current in ms (exponential
function).
  tau_syn_in double - Characteristic decrease time of the inhibitory synaptic current in ms
(exponential function).

Integration parameters
  gsl_error_tol  double - This parameter controls the admissible error of the GSL integrator.
                          Reduce it if NEST complains about numerical instabilities.

Author: Tanguy Fardet, modified from Marc-Oliver Gewaltig's implementation

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

References: Brette R and Gerstner W (2005) Adaptive Exponential Integrate-and-
  Fire Model as an Effective Description of Neuronal Activity.
  J Neurophysiol 94:3637-3642

SeeAlso: iaf_cond_alpha, aeif_cond_exp, aeif_cond_alpha
*/


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
extern "C" int aeif_psc_exp_ps_dynamics( double, const double*, double*, void* );

class aeif_psc_exp_ps : public Archiving_Node
{
public:
  /**
  * The constructor is only used to create the model prototype in the model manager.
  */
  aeif_psc_exp_ps();

  /**
  * The copy constructor is used to create model copies and instances of the model.
  * @node The copy constructor needs to initialize the parameters and the state.
  *       Initialization of buffers and interal variables is deferred to
  *       @c init_buffers_() and @c calibrate().
  */
  aeif_psc_exp_ps( const aeif_psc_exp_ps& );

  /**
  * Import sets of overloaded virtual functions.
  * This is necessary to ensure proper overload and overriding resolution.
  * @see http://www.gotw.ca/gotw/005.htm.
  */
  using Node::handle;
  using Node::handles_test_event;

  /**
  * Used to validate that we can send SpikeEvent to desired target:port.
  */
  port send_test_event( Node&, port, synindex, bool );

  /**
  * @defgroup mynest_handle Functions handling incoming events.
  * We tell nest that we can handle incoming events of various types by
  * defining @c handle() and @c connect_sender() for the given event.
  * @{
  */
  void handle( SpikeEvent& );         //! accept spikes
  void handle( CurrentEvent& );       //! accept input current
  void handle( DataLoggingRequest& ); //! allow recording with multimeter

  bool
  is_off_grid() const
  {
    return true;
  } // uses off_grid events

  port handles_test_event( SpikeEvent&, port );
  port handles_test_event( CurrentEvent&, port );
  port handles_test_event( DataLoggingRequest&, port );
  /** @} */

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  //! Reset parameters and state of neuron.

  //! Reset state of neuron.
  void init_state_( const Node& proto );

  //! Reset internal buffers of neuron.
  void init_buffers_();

  //! Initialize auxiliary quantities, leave parameters and state untouched.
  void calibrate();

  //! Take neuron through given time interval
  void update( const Time&, const long_t, const long_t );

  //! Find the precise time of network crossing
  void interpolate_( double&, double );

  //! Send spike and set refractoriness
  void spiking_( const long_t, const long_t, const double );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< aeif_psc_exp_ps >;
  friend class UniversalDataLogger< aeif_psc_exp_ps >;

  /**
  * Free parameters of the neuron.
  *
  * These are the parameters that can be set by the user through @c SetStatus.
  * They are initialized from the model prototype when the node is created.
  * Parameters do not change during calls to @c update() and are not reset by
  * @c ResetNetwork.
  *
  * @note Parameters_ need neither copy constructor nor @c operator=(), since
  *       all its members are copied properly by the default copy constructor
  *       and assignment operator. Important:
  *       - If Parameters_ contained @c Time members, you need to define the
  *         assignment operator to recalibrate all members of type @c Time . You
  *         may also want to define the assignment operator.
  *       - If Parameters_ contained members that cannot copy themselves, such
  *         as C-style arrays, you need to define the copy constructor and
  *         assignment operator to copy those members.
  */
  struct Parameters_
  {
    double_t V_peak_;  //!< Spike detection threshold in mV
    double_t V_reset_; //!< Reset Potential in mV
    double_t t_ref_;   //!< Refractory period in ms

    double_t g_L;        //!< Leak Conductance in nS
    double_t C_m;        //!< Membrane Capacitance in pF
    double_t E_L;        //!< Leak reversal Potential (aka resting potential) in mV
    double_t Delta_T;    //!< Slope faktor in ms.
    double_t tau_w;      //!< adaptation time-constant in ms.
    double_t a;          //!< Subthreshold adaptation in nS.
    double_t b;          //!< Spike-triggered adaptation in pA
    double_t V_th;       //!< Spike threshold in mV.
    double_t t_ref;      //!< Refractory period in ms.
    double_t tau_syn_ex; //!< Excitatory synaptic rise time.
    double_t tau_syn_in; //!< Excitatory synaptic rise time.
    double_t I_e;        //!< Intrinsic current in pA.
    int interpol_order;  //!< Interpolation order (from 1 to 3)

    double_t gsl_error_tol; //!< error bound for GSL integrator

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

public:
  /**
  * Dynamic state of the neuron.
  *
  * These are the state variables that are advanced in time by calls to
  * @c update(). In many models, some or all of them can be set by the user
  * through @c SetStatus. The state variables are initialized from the model
  * prototype when the node is created. State variables are reset by @c ResetNetwork.
  *
  * @note State_ need neither copy constructor nor @c operator=(), since
  *       all its members are copied properly by the default copy constructor
  *       and assignment operator. Important:
  *       - If State_ contained @c Time members, you need to define the
  *         assignment operator to recalibrate all members of type @c Time . You
  *         may also want to define the assignment operator.
  *       - If State_ contained members that cannot copy themselves, such
  *         as C-style arrays, you need to define the copy constructor and
  *         assignment operator to copy those members.
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
      I_EXC, // 1
      I_INH, // 2
      W,     // 3
      STATE_VEC_SIZE
    };

    double_t y_[ STATE_VEC_SIZE ];     //!< neuron state, must be C-array for GSL solver
    double_t y_old_[ STATE_VEC_SIZE ]; //!< old neuron state, must be C-array for GSL solver
    int_t r_;                          //!< number of refractory steps remaining
    double_t r_offset_; // offset on the refractory time if it is not a multiple of step_

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };

  /**
  * Buffers of the neuron.
  * Ususally buffers for incoming spikes and data logged for analog recorders.
  * Buffers must be initialized by @c init_buffers_(), which is called before
  * @c calibrate() on the first call to @c Simulate after the start of NEST,
  * ResetKernel or ResetNetwork.
  * @node Buffers_ needs neither constructor, copy constructor or assignment operator,
  *       since it is initialized by @c init_nodes_(). If Buffers_ has members that
  *       cannot destroy themselves, Buffers_ will need a destructor.
  */
  struct Buffers_
  {
    Buffers_( aeif_psc_exp_ps& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, aeif_psc_exp_ps& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< aeif_psc_exp_ps > logger_;

    /** buffers and sums up incoming spikes/currents */
    SliceRingBufferNew events_;
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
    double_t step_;          //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL

    /**
    * Input current injected by CurrentEvent.
    * This variable is used to transport the current applied into the
    * _dynamics function computing the derivative of the state vector.
    * It must be a part of Buffers_, since it is initialized once before
    * the first simulation, but not modified before later Simulate calls.
    */
    double_t I_stim_;
  };

  /**
  * Internal variables of the neuron.
  * These variables must be initialized by @c calibrate, which is called before
  * the first call to @c update() upon each call to @c Simulate.
  * @node Variables_ needs neither constructor, copy constructor or assignment operator,
  *       since it is initialized by @c calibrate(). If Variables_ has members that
  *       cannot destroy themselves, Variables_ will need a destructor.
  */
  struct Variables_
  {
    int_t RefractoryCounts_;
    double_t RefractoryOffset_;
  };

  /**
  * @defgroup Access functions for UniversalDataLogger.
  * @{
  */
  //! Read out the real membrane potential
  template < State_::StateVecElems elem >
  double_t
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }
  //! Read out the old state
  template < State_::StateVecElems elem >
  double_t
  get_y_old_elem_() const
  {
    return S_.y_old_[ elem ];
  }
  /** @} */

  /**
  * @defgroup pif_members Member variables of neuron model.
  * Each model neuron should have precisely the following four data members,
  * which are one instance each of the parameters, state, buffers and variables
  * structures. Experience indicates that the state and variables member should
  * be next to each other to achieve good efficiency (caching).
  * @note Devices require one additional data member, an instance of the @c Device
  *       child class they belong to.
  * @{
  */
  Parameters_ P_; //!< Free parameters.
  State_ S_;      //!< Dynamic state.
  Variables_ V_;  //!< Internal Variables
  Buffers_ B_;    //!< Buffers.

  //! Mapping of recordables names to access functions
  static RecordablesMap< aeif_psc_exp_ps > recordablesMap_;

  /** @} */
};

inline port
aeif_psc_exp_ps::send_test_event( Node& target, port receptor_type, synindex, bool )
{
  // You should usually not change the code in this function.
  // It confirms that the target of connection @c c accepts @c SpikeEvent on
  // the given @c receptor_type.
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
aeif_psc_exp_ps::handles_test_event( SpikeEvent&, port receptor_type )
{
  // You should usually not change the code in this function.
  // It confirms to the connection management system that we are able
  // to handle @c SpikeEvent on port 0. You need to extend the function
  // if you want to differentiate between input ports.
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline port
aeif_psc_exp_ps::handles_test_event( CurrentEvent&, port receptor_type )
{
  // You should usually not change the code in this function.
  // It confirms to the connection management system that we are able
  // to handle @c CurrentEvent on port 0. You need to extend the function
  // if you want to differentiate between input ports.
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline port
aeif_psc_exp_ps::handles_test_event( DataLoggingRequest& dlr, port receptor_type )
{
  // You should usually not change the code in this function.
  // It confirms to the connection management system that we are able
  // to handle @c DataLoggingRequest on port 0.
  // The function also tells the built-in UniversalDataLogger that this node
  // is recorded from and that it thus needs to collect data during simulation.
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
aeif_psc_exp_ps::get_status( DictionaryDatum& d ) const
{
  // get our own parameter and state data
  P_.get( d );
  S_.get( d );

  // get information managed by parent class
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
aeif_psc_exp_ps::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, ptmp );   // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif /* #ifdef HAVE_GSL_1_11 */
#endif /* #ifndef AEIF_PSC_EXP_PS_H */
