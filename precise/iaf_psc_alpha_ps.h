/*
 *  iaf_psc_alpha_ps.h
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

#ifndef IAF_PSC_ALPHA_PS_H
#define IAF_PSC_ALPHA_PS_H

// C++ includes:
#include <vector>

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
#include "ring_buffer.h"
#include "universal_data_logger.h"

// Includes from precise:
#include "slice_ring_buffer_new.h"

/*BeginDocumentation
Name: iaf_psc_alpha_ps - Leaky integrate-and-fire neuron
  with alpha-shape postsynaptic currents; implementing precise
  spikes, as well as a linear interpolation to find the "exact" time where the
  threshold was crossed, i.e. the spiking time.

Description:
iaf_psc_alpha_ps is an implementation of the leaky integrate-and-fire model 
neuron with alpha-shaped postsynaptic currents in the sense of [1]. This model 
implements precise spikes and a linear interpolation to find spike times more
precisely.

PSCs are normalized to a maximum amplitude equal to the synaptic weight.

This implementation handles neuronal dynamics in a locally
event-based manner with in coarse time grid defined by the minimum
delay in the network, see [1]. Incoming spikes are applied at the
precise moment of their arrival, while the precise time of outgoing
spikes is determined by interpolation once a threshold crossing has
been detected. Return from refractoriness occurs precisly at spike
time plus refractory period.

This implementation is more complex than the plain iaf_psc_alpha
neuron, but achieves much higher precision. In particular, it does not
suffer any binning of spike times to grid points. Depending on your
application, the precise-spiking application may provide superior overall
performance given an accuracy goal; see [1] for details. Dynamics is integrated
using the GSL library.

Remarks:
Please note that this node is capable of sending precise spike times
to target nodes (on-grid spike time plus offset). If this node is
connected to a spike_detector, the property "precise_times" of the
spike_detector has to be set to true in order to record the offsets
in addition to the on-grid spike times.

A further improvement of precise simulation is implemented in iaf_psc_exp_ps
based on [3].

Parameters:
The following parameters can be set in the status dictionary.

  V_m          double - Membrane potential in mV
  E_L          double - Resting membrane potential in mV.
  V_min        double - Absolute lower value for the membrane potential.
  C_m          double - Capacity of the membrane in pF
  g_L          double - Leak conductance in nS.
  t_ref        double - Duration of refractory period in ms.
  V_th         double - Spike threshold in mV.
  V_reset      double - Reset potential of the membrane in mV.
  tau_syn_exc  double - Rise time of the excitatory synaptic alpha function in ms
  tau_syn_inh  double - Rise time of the inhibitory synaptic alpha function in ms
  I_e          double - Constant external input current in pA.

References:
[1] Morrison A, Straube S, Plesser H E, & Diesmann M (2006) Exact Subthreshold
    Integration with Continuous Spike Times in Discrete Time Neural Network
    Simulations. To appear in Neural Computation.
[2] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
    systems with applications to neuronal modeling. Biologial Cybernetics
    81:381-402.
[3] Hanuschkin A, Kunkel S, Helias M, Morrison A & Diesmann M (2010)
    A general and efficient method for incorporating exact spike times in
    globally time-driven simulations Front Neuroinformatics, 4:113

Author: Tanguy Fardet, modified from Diesmann, Eppler, Morrison, Plesser, Straube

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

SeeAlso: iaf_psc_alpha, iaf_psc_alpha_presc, iaf_psc_exp_ps

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
extern "C" int iaf_psc_alpha_ps_dynamics( double, const double*, double*, void* );

/**
 * Leaky iaf neuron, alpha PSC synapses, canonical implementation.
 * @note Inherit privately from Node, so no classes can be derived
 * from this one.
 * @todo Implement current input in consistent way.
 */
class iaf_psc_alpha_ps : public Archiving_Node
{
  public:
    /** Basic constructor.
        This constructor should only be used by GenericModel to create
        model prototype instances.
    */
    iaf_psc_alpha_ps();

    /** Copy constructor.
        GenericModel::allocate_() uses the copy constructor to clone
        actual model instances from the prototype instance.

        @note The copy constructor MUST NOT be used to create nodes based
        on nodes that have been placed in the network.
    */
    iaf_psc_alpha_ps( const iaf_psc_alpha_ps& );

    /**
     * Import sets of overloaded virtual functions.
     * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
     */
    using Node::handle;
    using Node::handles_test_event;

    port send_test_event( Node&, rport, synindex, bool );

    port handles_test_event( SpikeEvent&, rport );
    port handles_test_event( CurrentEvent&, rport );
    port handles_test_event( DataLoggingRequest&, rport );

    void handle( SpikeEvent& );
    void handle( CurrentEvent& );
    void handle( DataLoggingRequest& );

    bool
    is_off_grid() const
    {
      return true;
    } // uses off_grid events

    void get_status( DictionaryDatum& ) const;
    void set_status( const DictionaryDatum& );

  private:
    /** @name Interface functions
     * @note These functions are private, so that they can be accessed
     * only through a Node*.
     */
    //@{
    void init_state_( const Node& proto );
    void init_buffers_();
    void calibrate();

    /**
     * Time Evolution Operator.
     *
     * update() promotes the state of the neuron from origin+from to origin+to.
     * It does so in steps of the resolution h.  Within each step, time is
     * advanced from event to event, as retrieved from the spike queue.
     *
     * Return from refractoriness is handled as a special event in the
     * queue, which is marked by a weight that is GSL_NAN.  This greatly simplifies
     * the code.
     *
     * For steps, during which no events occur, the precomputed propagator matrix
     * is used.  For other steps, the propagator matrix is computed as needed.
     *
     * While the neuron is refractory, membrane potential (y3_) is
     * clamped to U_reset_.
     */
    
    //! Take neuron through given time interval
    void update( const Time&, const long_t, const long_t );

    //! Find the precise time of network crossing
    void interpolate_( double&, double );

    //! Send spike and set refractoriness
    void spiking_( const long_t, const long_t, const double );

    // The next two classes need to be friends to access the State_ class/member
    friend class RecordablesMap< iaf_psc_alpha_ps >;
    friend class UniversalDataLogger< iaf_psc_alpha_ps >;

    // ----------------------------------------------------------------

    /**
     * Independent parameters of the model.
     */

    struct Parameters_
    {
       double_t V_reset_; //!< Reset Potential in mV
       double_t t_ref_;   //!< Refractory period in ms

       double_t g_L;        //!< Leak Conductance in nS
       double_t C_m;        //!< Membrane Capacitance in pF
       double_t E_ex;       //!< Excitatory reversal Potential in mV
       double_t E_in;       //!< Inhibitory reversal Potential in mV
       double_t E_L;        //!< Leak reversal Potential (aka resting potential) in mV
       double_t V_th;       //!< Spike threshold in mV.
       double_t t_ref;      //!< Refractory period in ms.
       double_t tau_syn_exc; //!< Excitatory synaptic rise time.
       double_t tau_syn_inh; //!< Excitatory synaptic rise time.
       double_t I_e;        //!< Intrinsic current in pA.

       double_t gsl_error_tol; //!< error bound for GSL integrator

       Parameters_(); //!< Sets default parameter values

       void get( DictionaryDatum& ) const; //!< Store current values in dictionary
       void set( const DictionaryDatum& ); //!< Set values from dicitonary
    };

    // ----------------------------------------------------------------

  public:
    /**
     * State variables of the model.
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
          STATE_VEC_SIZE
       };

       double_t y_[ STATE_VEC_SIZE ]; //!< neuron state, must be C-array for GSL solver
       double_t y_old_[ STATE_VEC_SIZE ]; //!< old neuron state, must be C-array for GSL solver
       int_t r_;                      //!< number of refractory steps remaining
       double_t r_offset_;      // offset on the refractory time if it is not a multiple of step_

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
       Buffers_( iaf_psc_alpha_ps& );                  //!<Sets buffer pointers to 0
       Buffers_( const Buffers_&, iaf_psc_alpha_ps& ); //!<Sets buffer pointers to 0

       //! Logger for all analog data
       nest::UniversalDataLogger< iaf_psc_alpha_ps > logger_;

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
      /** initial value to normalise excitatory psc */
      double_t I0_ex_; //!< e / tau_syn_exc

      /** initial value to normalise inhibitory psc */
      double_t I0_in_; //!< e / tau_syn_inh

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
    static RecordablesMap< iaf_psc_alpha_ps > recordablesMap_;

    /** @} */
};

inline port
nest::iaf_psc_alpha_ps::send_test_event( Node& target,
  port receptor_type,
  synindex,
  bool )
{
  // You should usually not change the code in this function.
  // It confirms that the target of connection @c c accepts @c SpikeEvent on
  // the given @c receptor_type.
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
nest::iaf_psc_alpha_ps::handles_test_event( SpikeEvent&, port receptor_type )
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
nest::iaf_psc_alpha_ps::handles_test_event( CurrentEvent&, port receptor_type )
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
nest::iaf_psc_alpha_ps::handles_test_event( DataLoggingRequest& dlr, port receptor_type )
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
iaf_psc_alpha_ps::get_status( DictionaryDatum& d ) const
{
  // get our own parameter and state data
  P_.get( d );
  S_.get( d );

  // get information managed by parent class
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_alpha_ps::set_status( const DictionaryDatum& d )
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
#endif /* #ifndef IAF_PSC_ALPHA_PS_H */
