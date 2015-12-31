/*
 *  iaf_cond_alpha_mc_kinetics.h
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

#ifndef IAF_COND_ALPHA_MC_KINETICS_H
#define IAF_COND_ALPHA_MC_KINETICS_H

#include "config.h"

#ifdef HAVE_GSL

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"
#include "recordables_map.h"
#include "dictdatum.h"
#include "name.h"

#include <vector>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

/* BeginDocumentation
Name: iaf_cond_alpha_mc_kinetics - PROTOTYPE Multi-compartment conductance-based
leaky integrate-and-fire neuron model with calcium spike modeled using first order kinetics.

Description:
THIS MODEL IS A PROTOTYPE FOR ILLUSTRATION PURPOSES. IT IS NOT YET
FULLY TESTED. USE AT YOUR OWN PERIL!

iaf_cond_alpha_mc_kinetics is an implementation of a multi-compartment spiking
neuron using IAF dynamics with conductance-based synapses. It serves
mainly to illustrate the implementation of multicompartment models in
NEST.

The model has three compartments: soma, proximal and distal dendrite,
labeled as s, p, and d, respectively. Compartments are connected through
passive conductances as follows

C_m.s d/dt V_m.s = ... - g_sp ( V_m.s - V_m.p )

C_m.p d/dt V_m.p = ... - g_sp ( V_m.p - V_m.s ) - g_pd ( V_m.p - V_m.d )

C_m.d d/dt V_m.d = ...                          - g_pd ( V_m.d - V_m.p ) + ICa

A spike is fired when the somatic membrane potential exceeds threshold,
V_m.s >= V_th. Upon threshold crossing, somatic membrane potential is set to
a maximum potential, V_m.s == V_max, and somatic leak set to a larger leak value
for the refractory period. Depending on value of the reset flag, the somatic
membrane potential maybe reset to reset value at end of refractory period.
To emulate backpropagating action potential, an alpha shaped current is
introduced 1ms and 2ms after spike at proximal and distal compartment respectively.
Dendritic membrane potentials are not manipulated after a spike.
The spike threshold is adaptive, whereby it jumps by a jump value upon crossing,
and then decays exponentially back to baseline value.

There is one excitatory and one inhibitory conductance-based synapse
onto each compartment, with alpha-function time course. The alpha
function is normalised such that an event of weight 1.0 results in a
peak current of 1 nS at t = tau_syn. Each compartment can also receive
current input from a current generator, and an external (rheobase)
current can be set for each compartment.

Synapses, including those for injection external currents, are addressed through
the receptor types given in the receptor_types entry of the state dictionary. Note
that in contrast to the single-compartment iaf_cond_alpha model, all synaptic
weights must be positive numbers!

Depending on value of active flag, calcium spike maybe triggered depending on its kinetics dynamics.
The spike is modeled using first order dynamics, whereby

ICa_ = M_CA * H_CA * G_Ca * (E_Ca - V_m.d)
d/dt M_CA = (minf - M_CA) / tau_m
d/dt H_CA = (hinf - H_CA) / tau_h

and

minf = 1.0/( 1.0 + exp ^ ( (V_m.d - half_m) * -slope_m ) )
hinf = 1.0/( 1.0 + exp ^ ( (V_m.d - half_h) * -slope_h ) )


Parameters:
The following parameters can be set in the status dictionary. Parameters
for each compartment are collected in a sub-dictionary; these sub-dictionaries
are called "soma", "proximal", and "distal", respectively. In the list below,
these parameters are marked with an asterisk.

V_m*         double - Membrane potential in mV
E_L*         double - Leak reversal potential in mV.
C_m*         double - Capacity of the membrane in pF
E_ex*        double - Excitatory reversal potential in mV.
E_in*        double - Inhibitory reversal potential in mV.
g_L*         double - Leak conductance in nS;
tau_syn_ex*  double - Rise time of the excitatory synaptic alpha function in ms.
tau_syn_in*  double - Rise time of the inhibitory synaptic alpha function in ms.
I_e*         double - Constant input current in pA.
t_L*         double - Leak during refractory period in nS
nt_L*        double - Leak at other times in ns
tau_curAP*   double - Time constant of active current at each compartment after action potential in
pA
amp_curAP*   double - Amplitude of active current at each compartment after action potential in pA

g_sp         double - Conductance connecting soma and proximal dendrite, in nS.
g_pd         double - Conductance connecting proximal and distal dendrite, in nS.
t_ref        double - Duration of refractory period in ms.
V_th         double - Spike threshold in mV.
V_reset      double - Reset potential of the membrane in mV.
V_max        double - Peak voltage at spike in mV
jump_Th      double - Jump in adaptive threshold upon spike in mV
tau_Th       double - Time constant for adaptive threshold in ms
act_flag     double - flag to turn on calcium spikes
reset_flag   double - flag to set somatic membrane potential to reset value at end of refractory
period
E_Ca         double - Reversal potential for calcium spike in mV
G_Ca         double - Maximal conductance for calcium spike in nS
tau_m        double - Time constant for activation M in ms
tau_h        double - Time constant for inactivation H in ms
half_m       double - Voltage for half activation of M-inf in mV
half_h       double - Voltage for half activation of H-inf in mV
slope_m      double - Slope of M-inf in mV-1
slope_h      double - Slope of H-inf in mV-1

Example:
See examples/nest/mc_neuron.py.

Remark:
This is a prototype for illustration which has undergone only limited testing.
Details of the implementation and user-interface will likely change.
USE AT YOUR OWN PERIL!

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

References:

Meffin, H., Burkitt, A. N., & Grayden, D. B. (2004). An analytical
model for the large, fluctuating synaptic conductance state typical of
neocortical neurons in vivo. J.  Comput. Neurosci., 16, 159–175.

Bernander, O ., Douglas, R. J., Martin, K. A. C., & Koch, C. (1991).
Synaptic background activity influences spatiotemporal integration in
single pyramidal cells.  Proc. Natl. Acad. Sci. USA, 88(24),
11569–11573.

Chua, Y., Morrison, A., & Moritz, H. (2015).
Modeling the calcium spike as a threshold triggered fixed waveform for
synchronous inputs in the fluctuation regime. Frontiers in Computational Neuroscience.,
9(00091).

Author: Plesser (multicompartment neuron), Yansong Chua (calcium spike added in neuron)

SeeAlso: iaf_cond_alpha, iaf_cond_alpha_mc
*/

namespace nest
{
/**
 * Function computing right-hand side of ODE for GSL solver.
 * @note Must be declared here so we can befriend it in class.
 * @note Must have C-linkage for passing to GSL.
 * @note No point in declaring it inline, since it is called
 *       through a function pointer.
 */
extern "C" int iaf_cond_alpha_mc_kinetics_dynamics( double, const double*, double*, void* );

/**
 * @note All parameters that occur for both compartments
 *       and dendrite are stored as C arrays, with index 0 being soma.
 */
class iaf_cond_alpha_mc_kinetics : public Archiving_Node
{

  // Boilerplate function declarations --------------------------------

public:
  iaf_cond_alpha_mc_kinetics();
  iaf_cond_alpha_mc_kinetics( const iaf_cond_alpha_mc_kinetics& );
  ~iaf_cond_alpha_mc_kinetics();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
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
  void update( Time const&, const long_t, const long_t );

  // Enumerations and constants specifying structure and properties ----

  //! Compartments, NCOMP is number
  enum Compartments_
  {
    SOMA = 0,
    PROX,
    DIST,
    NCOMP
  };

  /**
   * Minimal spike receptor type.
   * @note Start with 1 so we can forbid port 0 to avoid accidental
   *       creation of connections with no receptor type set.
   */
  static const port MIN_SPIKE_RECEPTOR = 1;

  /**
   * Spike receptors.
   */
  enum SpikeSynapseTypes
  {
    SOMA_EXC = MIN_SPIKE_RECEPTOR,
    SOMA_INH,
    PROX_EXC,
    PROX_INH,
    DIST_EXC,
    DIST_INH,
    SUP_SPIKE_RECEPTOR
  };

  static const size_t NUM_SPIKE_RECEPTORS = SUP_SPIKE_RECEPTOR - MIN_SPIKE_RECEPTOR;

  /**
   * Minimal current receptor type.
   *  @note Start with SUP_SPIKE_RECEPTOR to avoid any overlap and
   *        accidental mix-ups.
   */
  static const port MIN_CURR_RECEPTOR = SUP_SPIKE_RECEPTOR;

  /**
   * Current receptors.
   */
  enum CurrentSynapseTypes
  {
    I_SOMA = MIN_CURR_RECEPTOR,
    I_PROX,
    I_DIST,
    SUP_CURR_RECEPTOR
  };

  static const size_t NUM_CURR_RECEPTORS = SUP_CURR_RECEPTOR - MIN_CURR_RECEPTOR;

  // Friends --------------------------------------------------------

  friend int iaf_cond_alpha_mc_kinetics_dynamics( double, const double*, double*, void* );

  friend class RecordablesMap< iaf_cond_alpha_mc_kinetics >;
  friend class UniversalDataLogger< iaf_cond_alpha_mc_kinetics >;


  // Parameters ------------------------------------------------------

  /**
   * Independent parameters of the model.
   * These parameters must be passed to the iteration function that
   * is passed to the GSL ODE solvers. Since the iteration function
   * is a C++ function with C linkage, the parameters can be stored
   * in a C++ struct with member functions, as long as we just pass
   * it by void* from C++ to C++ function. The struct must be public,
   * though, since the iteration function is a function with C-linkage,
   * whence it cannot be a member function of iaf_cond_alpha_mc_kinetics.
   * @note One could achieve proper encapsulation by an extra level
   *       of indirection: Define the iteration function as a member
   *       function, plus an additional wrapper function with C linkage.
   *       Then pass a struct containing a pointer to the node and a
   *       pointer-to-member-function to the iteration function as void*
   *       to the wrapper function. The wrapper function can then invoke
   *       the iteration function on the node (Stroustrup, p 418). But
   *       this appears to involved, and the extra indirections cost.
   */
  struct Parameters_
  {
    double_t V_th;    //!< Threshold Potential in mV
    double_t V_reset; //!< Reset Potential in mV
    double_t t_ref;   //!< Refractory period in ms
    // changes
    double_t V_max;      //!< Peak voltage at spike in mV
    double_t E_Ca;       //!< Reversal potential for calcium spike in mV
    double_t G_Ca;       //!< Maximal conductance for calcium spike in nS
    double_t tau_m;      //!< Time constant for activation M in ms
    double_t tau_h;      //!< Time constant for inactivation H in ms
    double_t half_m;     //!< Voltage for half activation of M-inf in mV
    double_t half_h;     //!< Voltage for half activation of H-inf in mV
    double_t slope_m;    //!< Slope of M-inf in mV-1
    double_t slope_h;    //!< Slope of H-inf in mV-1
    double_t jump_Th;    //!< Jump in adaptive threshold upon spike in mV
    double_t tau_Th;     //!< Time constant for adaptive threshold in ms
    double_t act_flag;   //!< flag to turn on calcium spikes
    double_t reset_flag; //!< flag to turn on calcium spikes

    double_t g_conn[ NCOMP - 1 ]; //!< Conductances connecting compartments, in nS
    double_t t_L[ NCOMP ];        //!< Leak during refractory period in nS
    double_t nt_L[ NCOMP ];       //!< Leak at other times in ns

    double_t g_L[ NCOMP ];      //!< Leak Conductance in nS
    double_t C_m[ NCOMP ];      //!< Membrane Capacitance in pF
    double_t E_ex[ NCOMP ];     //!< Excitatory reversal Potential in mV
    double_t E_in[ NCOMP ];     //!< Inhibitory reversal Potential in mV
    double_t E_L[ NCOMP ];      //!< Leak reversal Potential (aka resting potential) in mV
    double_t tau_synE[ NCOMP ]; //!< Synaptic Time Constant Excitatory Synapse in ms
    double_t tau_synI[ NCOMP ]; //!< Synaptic Time Constant for Inhibitory Synapse in ms
    double_t I_e[ NCOMP ];      //!< Constant Current in pA
    // changes
    double_t
      tau_curAP[ NCOMP ]; //!< Time constant of active current at each compartment after AP in pA
    double_t amp_curAP[ NCOMP ]; //!< Amplitude of active current at each compartment after AP in pA

    Parameters_();                                //!< Sets default parameter values
    Parameters_( const Parameters_& );            //!< needed to copy C-arrays
    Parameters_& operator=( const Parameters_& ); //!< needed to copy C-arrays

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };


  // State variables  ------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor and assignment operator required because
   *       of C-style array.
   */
public:
  struct State_
  {

    /**
     * Elements of state vector.
     * For the multicompartmental case here, these are offset values.
     * The state variables are stored in contiguous blocks for each
     * compartment, beginning with the soma.
     */
    enum StateVecElems_
    {
      V_M = 0,
      DG_EXC,
      G_EXC,
      DG_INH,
      G_INH,
      M_CA,
      H_CA,
      DI_AP,
      I_AP,
      STATE_VEC_COMPS
    };

    //! total size of state vector
    static const size_t STATE_VEC_SIZE = STATE_VEC_COMPS * NCOMP;

    //! neuron state, must be C-array for GSL solver
    double_t y_[ STATE_VEC_SIZE ];
    int_t r_; //!< number of refractory steps remaining
    // changes
    int_t numCa_;  // num of calcium spikes
    double_t th_;  // adaptive spike threshold
    double_t ICa_; // calcium current

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_& );

    /**
     * Compute linear index into state array from compartment and element.
     * @param comp compartment index
     * @param elem elemet index
     * @note compartment argument is not of type Compartments_, since looping
     *       over enumerations does not work.
     */
    static size_t
    idx( size_t comp, StateVecElems_ elem )
    {
      return comp * STATE_VEC_COMPS + elem;
    }
  };

private:
  // Internal buffers --------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_cond_alpha_mc_kinetics& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, iaf_cond_alpha_mc_kinetics& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< iaf_cond_alpha_mc_kinetics > logger_;

    /** buffers and sums up incoming spikes/currents
     *  @note Using STL vectors here to ensure initialization.
     */
    std::vector< RingBuffer > spikes_;
    std::vector< RingBuffer > currents_;

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
     * Input currents injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double_t I_stim_[ NCOMP ]; //!< External Stimulus in pA
  };

  // Internal variables ---------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** initial value to normalise excitatory synaptic conductance */
    double_t PSConInit_E_[ NCOMP ];

    /** initial value to normalise inhibitory synaptic conductance */
    double_t PSConInit_I_[ NCOMP ];

    /** initial value to normalise current after AP */
    double_t PSConInit_AP_[ NCOMP ];

    int_t RefractoryCounts_;

    // changes
    /** initial value to normalise calcium spike */
    double_t PSConInit_Ca_;

    int_t RefractoryCountsCa_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  /**
   * Read out state vector elements, used by UniversalDataLogger
   * First template argument is component "name", second compartment "name".
   */
  template < State_::StateVecElems_ elem, Compartments_ comp >
  double_t
  get_y_elem_() const
  {
    return S_.y_[ S_.idx( comp, elem ) ];
  }

  //! Read out number of refractory steps, used by UniversalDataLogger
  double_t
  get_r_() const
  {
    return Time::get_resolution().get_ms() * S_.r_;
  }

  // changes
  //! Read out number of threshold, used by UniversalDataLogger
  double_t
  get_th_() const
  {
    return S_.th_;
  }
  double_t
  get_ca_() const
  {
    return S_.numCa_;
  }
  double_t
  get_ica_() const
  {
    return S_.ICa_;
  }


  // Data members ----------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Table of compartment names
  static std::vector< Name > comp_names_;

  //! Dictionary of receptor types, leads to seg fault on exit, see #328
  // static DictionaryDatum receptor_dict_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< iaf_cond_alpha_mc_kinetics > recordablesMap_;
};

inline port
iaf_cond_alpha_mc_kinetics::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_cond_alpha_mc_kinetics::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type < MIN_SPIKE_RECEPTOR || receptor_type >= SUP_SPIKE_RECEPTOR )
  {
    if ( receptor_type < 0 || receptor_type >= SUP_CURR_RECEPTOR )
      throw UnknownReceptorType( receptor_type, get_name() );
    else
      throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }
  return receptor_type - MIN_SPIKE_RECEPTOR;
}

inline port
iaf_cond_alpha_mc_kinetics::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type < MIN_CURR_RECEPTOR || receptor_type >= SUP_CURR_RECEPTOR )
  {
    if ( receptor_type >= 0 && receptor_type < MIN_CURR_RECEPTOR )
      throw IncompatibleReceptorType( receptor_type, get_name(), "CurrentEvent" );
    else
      throw UnknownReceptorType( receptor_type, get_name() );
  }
  return receptor_type - MIN_CURR_RECEPTOR;
}

inline port
iaf_cond_alpha_mc_kinetics::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    if ( receptor_type < 0 || receptor_type >= SUP_CURR_RECEPTOR )
      throw UnknownReceptorType( receptor_type, get_name() );
    else
      throw IncompatibleReceptorType( receptor_type, get_name(), "DataLoggingRequest" );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_cond_alpha_mc_kinetics::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  /**
   * @todo dictionary construction should be done only once for
   * static member in default c'tor, but this leads to
   * a seg fault on exit, see #328
   */
  DictionaryDatum receptor_dict_ = new Dictionary();
  ( *receptor_dict_ )[ Name( "soma_exc" ) ] = SOMA_EXC;
  ( *receptor_dict_ )[ Name( "soma_inh" ) ] = SOMA_INH;
  ( *receptor_dict_ )[ Name( "soma_curr" ) ] = I_SOMA;

  ( *receptor_dict_ )[ Name( "proximal_exc" ) ] = PROX_EXC;
  ( *receptor_dict_ )[ Name( "proximal_inh" ) ] = PROX_INH;
  ( *receptor_dict_ )[ Name( "proximal_curr" ) ] = I_PROX;

  ( *receptor_dict_ )[ Name( "distal_exc" ) ] = DIST_EXC;
  ( *receptor_dict_ )[ Name( "distal_inh" ) ] = DIST_INH;
  ( *receptor_dict_ )[ Name( "distal_curr" ) ] = I_DIST;

  ( *d )[ names::receptor_types ] = receptor_dict_;
}

inline void
iaf_cond_alpha_mc_kinetics::set_status( const DictionaryDatum& d )
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
#endif // IAF_COND_ALPHA_MC_KINETICS_H
