/*
 *  iaf_cond_exp.h
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

#ifndef IAF_COND_EXP_H
#define IAF_COND_EXP_H

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

/* BeginDocumentation
Name: iaf_cond_exp - Simple conductance based leaky integrate-and-fire neuron
                     model.

Description:
iaf_cond_exp is an implementation of a spiking neuron using IAF dynamics with
conductance-based synapses. Incoming spike events induce a post-synaptic change
of conductance modelled by an exponential function. The exponential function
is normalised such that an event of weight 1.0 results in a peak conductance of
1 nS.

Parameters:
The following parameters can be set in the status dictionary.

V_m        double - Membrane potential in mV
E_L        double - Leak reversal potential in mV.
C_m        double - Capacity of the membrane in pF
t_ref      double - Duration of refractory period in ms.
V_th       double - Spike threshold in mV.
V_reset    double - Reset potential of the membrane in mV.
E_ex       double - Excitatory reversal potential in mV.
E_in       double - Inhibitory reversal potential in mV.
g_L        double - Leak conductance in nS;
tau_syn_ex double - Time constant of the excitatory synaptic exponential
                    function in ms.
tau_syn_in double - Time constant of the inhibitory synaptic exponential
                    function in ms.
I_e        double - Constant external input current in pA.

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

References:

Meffin, H., Burkitt, A. N., & Grayden, D. B. (2004). An analytical
model for the large, fluctuating synaptic conductance state typical of
neocortical neurons in vivo. J.  Comput. Neurosci., 16, 159-175.

Author: Sven Schrader

SeeAlso: iaf_psc_delta, iaf_psc_exp, iaf_cond_exp
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
extern "C" int iaf_cond_exp_dynamics( double, const double*, double*, void* );

class iaf_cond_exp : public Archiving_Node
{

public:
  iaf_cond_exp();
  iaf_cond_exp( const iaf_cond_exp& );
  ~iaf_cond_exp();

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
  friend int iaf_cond_exp_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_cond_exp >;
  friend class UniversalDataLogger< iaf_cond_exp >;

private:
  // ----------------------------------------------------------------

  //! Model parameters
  struct Parameters_
  {
    double V_th_;    //!< Threshold Potential in mV
    double V_reset_; //!< Reset Potential in mV
    double t_ref_;   //!< Refractory period in ms
    double g_L;      //!< Leak Conductance in nS
    double C_m;      //!< Membrane Capacitance in pF
    double E_ex;     //!< Excitatory reversal Potential in mV
    double E_in;     //!< Inhibitory reversal Potential in mV
    double E_L;      //!< Leak reversal Potential (aka resting potential) in mV
    double tau_synE; //!< Synaptic Time Constant Excitatory Synapse in ms
    double tau_synI; //!< Synaptic Time Constant for Inhibitory Synapse in ms
    double I_e;      //!< Constant Current in pA

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

    //! Symbolic indices to the elements of the state vector y
    enum StateVecElems
    {
      V_M = 0,
      G_EXC,
      G_INH,
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    int r_; //!< number of refractory steps remaining

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_cond_exp& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, iaf_cond_exp& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< iaf_cond_exp > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
    RingBuffer spike_inh_;
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
    int RefractoryCounts_;
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
  static RecordablesMap< iaf_cond_exp > recordablesMap_;
};


inline port
nest::iaf_cond_exp::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_cond_exp::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_cond_exp::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_cond_exp::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


inline void
iaf_cond_exp::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_cond_exp::set_status( const DictionaryDatum& d )
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

#endif // HAVE_GSL
#endif // IAF_COND_EXP_H
