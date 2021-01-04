/*
 *  hh_cond_exp_traub.h
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

#ifndef HH_COND_EXP_TRAUB_H
#define HH_COND_EXP_TRAUB_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C includes:
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
extern "C" int hh_cond_exp_traub_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: neuron, Hodgkin-Huxley, conductance-based

Short description
+++++++++++++++++

Hodgkin-Huxley model for Brette et al (2007) review

Description
+++++++++++

hh_cond_exp_traub is an implementation of a modified Hodgkin-Huxley model.

This model was specifically developed for a major review of simulators [1]_,
based on a model of hippocampal pyramidal cells by Traub and Miles [2]_.
The key differences between the current model and the model in [2]_ are:

- This model is a point neuron, not a compartmental model.
- This model includes only I_Na and I_K, with simpler I_K dynamics than
  in [2]_, so it has only three instead of eight gating variables;
  in particular, all Ca dynamics have been removed.
- Incoming spikes induce an instantaneous conductance change followed by
  exponential decay instead of activation over time.

This model is primarily provided as reference implementation for hh_coba
example of the Brette et al (2007) review. Default parameter values are chosen
to match those used with NEST 1.9.10 when preparing data for [1]_. Code for all
simulators covered is available from ModelDB [3]_.

Note:
In this model, a spike is emitted if :math:`V_m \geq V_T + 30` mV and :math:`V_m`
has fallen during the current time step.

To avoid that this leads to multiple spikes during the falling flank of a
spike, it is essential to chose a sufficiently long refractory period.
Traub and Miles used  :math:`t_{ref} = 3` ms ([2]_, p 118), while we used
:math:`t_{ref} = 2` ms in [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=========== ======  =========================================================
V_m          mV     Membrane potential
V_T          mV     Voltage offset that controls dynamics. For default
                    parameters, V_T = -63mV results in a threshold around
                    -50mV.
E_L          mV     Leak reversal potential
C_m          pF     Capacity of the membrane
g_L          nS     Leak conductance
tau_syn_ex   ms     Time constant of the excitatory synaptic exponential
                    function
tau_syn_in   ms     Time constant of the inhibitory synaptic exponential
                    function
t_ref        ms     Duration of refractory period (see Note).
E_ex         mV     Excitatory synaptic reversal potential
E_in         mV     Inhibitory synaptic reversal potential
E_Na         mV     Sodium reversal potential
g_Na         nS     Sodium peak conductance
E_K          mV     Potassium reversal potential
g_K          nS     Potassium peak conductance
I_e          pA     External input current
=========== ======  =========================================================

References
++++++++++

.. [1] Brette R et al. (2007). Simulation of networks of spiking neurons: A
       review of tools and strategies. Journal of Computational Neuroscience
       23:349-98. DOI: https://doi.org/10.1007/s10827-007-0038-6
.. [2] Traub RD and Miles R (1991). Neuronal networks of the hippocampus.
       Cambridge University Press, Cambridge UK.
.. [3] http://modeldb.yale.edu/83319

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

hh_psc_alpha

EndUserDocs */

class hh_cond_exp_traub : public ArchivingNode
{

public:
  hh_cond_exp_traub();
  hh_cond_exp_traub( const hh_cond_exp_traub& );
  ~hh_cond_exp_traub();

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
  friend int hh_cond_exp_traub_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< hh_cond_exp_traub >;
  friend class UniversalDataLogger< hh_cond_exp_traub >;

private:
  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    double g_Na; //!< Sodium Conductance in nS
    double g_K;  //!< Potassium Conductance in nS
    double g_L;  //!< Leak Conductance in nS
    double C_m;  //!< Membrane Capacitance in pF

    double E_Na; //!< Sodium Reversal Potential in mV
    double E_K;  //!< Potassium Reversal Potential in mV
    double E_L;  //!< Leak Reversal Potential in mV

    //! Voltage offset for dynamics (adjusts threshold to around -50 mV)
    double V_T;

    double E_ex;     //!< Excitatory reversal Potential in mV
    double E_in;     //!< Inhibitory reversal Potential in mV
    double tau_synE; //!< Synaptic Time Constant Excitatory Synapse in ms
    double tau_synI; //!< Synaptic Time Constant Inhibitory Synapse in ms
    double t_ref_;   //!< Refractory time in ms
    double I_e;      //!< External Current in pA

    Parameters_();

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    //! Symbolic indices to the elements of the state vector y
    enum StateVecElems
    {
      V_M = 0,
      HH_M,  // 1
      HH_H,  // 2
      HH_N,  // 3
      G_EXC, // 4
      G_INH, // 5
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    int r_; //!< number of refractory steps remaining

    State_( const Parameters_& p );
    State_( const State_& s );

    State_& operator=( const State_& s );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    int refractory_counts_;
    double U_old_; // for spike-detection
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( hh_cond_exp_traub& ); //!<Sets buffer pointers to 0
    //! Sets buffer pointers to 0
    Buffers_( const Buffers_&, hh_cond_exp_traub& );

    //! Logger for all analog data
    UniversalDataLogger< hh_cond_exp_traub > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
    RingBuffer spike_inh_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

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

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< hh_cond_exp_traub > recordablesMap_;
};

inline port
hh_cond_exp_traub::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
hh_cond_exp_traub::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
hh_cond_exp_traub::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
hh_cond_exp_traub::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
hh_cond_exp_traub::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  def< double >( d, names::t_spike, get_spiketime_ms() );
}

inline void
hh_cond_exp_traub::set_status( const DictionaryDatum& d )
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

  calibrate();
}

} // namespace


#endif // HAVE_GSL
#endif // HH_COND_EXP_TRAUB_H
