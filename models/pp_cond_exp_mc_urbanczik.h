/*
 *  pp_cond_exp_mc_urbanczik.h
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

#ifndef PP_COND_EXP_MC_URBANCZIK_H
#define PP_COND_EXP_MC_URBANCZIK_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C++ includes:
#include <vector>

// C includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "random_generators.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"
#include "urbanczik_archiving_node.h"
#include "urbanczik_archiving_node_impl.h"

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

namespace nest
{
/**
 * Function computing right-hand side of ODE for GSL solver.
 * @note Must be declared here so we can befriend it in class.
 * @note Must have C-linkage for passing to GSL.
 * @note No point in declaring it inline, since it is called
 *       through a function pointer.
 */
extern "C" int pp_cond_exp_mc_urbanczik_dynamics( double, const double*, double*, void* );

/** @BeginDocumentation
Name: pp_cond_exp_mc_urbanczik_parameters - Helper class for pp_cond_exp_mc_urbanczik

Description:
pp_cond_exp_mc_urbanczik_parameters is a helper class for the pp_cond_exp_mc_urbanczik neuron model
that contains all parameters of the model that are needed to compute the weight changes of a
connected urbanczik_synapse in the base class UrbanczikArchivingNode.

Author: Jonas Stapmanns, David Dahmen, Jan Hahne

SeeAlso: pp_cond_exp_mc_urbanczik
*/
class pp_cond_exp_mc_urbanczik_parameters
{
  friend class pp_cond_exp_mc_urbanczik;
  friend class UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >;

private:
  //! Compartments, NCOMP is number
  enum Compartments_
  {
    SOMA = 0,
    DEND,
    NCOMP
  };

  double phi_max;    //!< Parameter of the rate function
  double rate_slope; //!< Parameter of the rate function
  double beta;       //!< Parameter of the rate function
  double theta;      //!< Parameter of the rate function
  double phi( double u );
  double h( double u );

public:
  // The Urbanczik parameters need to be public within this class as they are passed to the GSL solver
  double g_conn[ NCOMP ];     //!< Conductances connecting compartments in nS
  double g_L[ NCOMP ];        //!< Leak Conductance in nS
  double C_m[ NCOMP ];        //!< Capacity of the membrane in pF
  double E_L[ NCOMP ];        //!< Reversal Potential in mV
  double tau_syn_ex[ NCOMP ]; //!< Rise time of excitatory synaptic conductance in ms
  double tau_syn_in[ NCOMP ]; //!< Rise time of inhibitory synaptic conductance in ms
};

/* BeginUserDocs: neuron, point process, conductance-based

Short description
+++++++++++++++++

Two-compartment point process neuron with conductance-based synapses

Description
+++++++++++

pp_cond_exp_mc_urbanczik is an implementation of a two-compartment spiking
point process neuron with conductance-based synapses as it is used
in [1]_. It is capable of connecting to an :doc:`Urbanczik synapse
<urbanczik_synapse>`.

The model has two compartments: soma and dendrite, labeled as s and p,
respectively. Each compartment can receive spike events and current input
from a current generator. Additionally, an external (rheobase) current can be
set for each compartment.

Synapses, including those for injection external currents, are addressed through
the receptor types given in the receptor_types entry of the state dictionary.
Note that in contrast to the single-compartment models, all
synaptic weights must be positive numbers! The distinction between excitatory
and inhibitory synapses is made explicitly by specifying the receptor type of
the synapse. For example, receptor_type=dendritic_exc results in an excitatory
input and receptor_type=dendritic_inh results in an inhibitory input to the
dendritic compartment.

.. _multicompartment-models:

Multicompartment models and synaptic delays
+++++++++++++++++++++++++++++++++++++++++++

Note that in case of multicompartment models that represent the dendrite
explicitly, the interpretation of the synaptic delay in NEST requires careful
consideration. In NEST, the delay is at least one simulation time step and is
assumed to be located entirely at the postsynaptic side. For point neurons, it
represents the time it takes for an incoming spike to travel along the
postsynaptic dendrite before it reaches the soma, see :ref:`panel a)
<fig-multicompartment>`. Conversely, if the synaptic weight depends on the
state of the postsynaptic neuron, the delay also represents the time it takes
for the information on the state to propagate back through the dendrite to the
synapse.

For multicompartment models in NEST, this means the delay is positioned directly
behind the incoming synapse, that is, before the first dendritic compartment on the
postsynaptic side, see :ref:`panel b) <fig-multicompartment>`. Therefore, the
delay specified in the synapse model does *not* account for any delay that might
be associated with information traveling through the explicitly modeled
dendritic compartments.

In the :ref:`Urbanczik synapse <urbanczik_synapse>`, the change of the synaptic
weight is driven by an error signal, which is the difference between the firing
rate of the soma (derived from the somatic spike train :math:`S_{post}`) and the
dendritic prediction of the firing rate of the soma (derived from the dendritic
membrane potential :math:`V`). The original publication [1]_ does not assume any
delay in the interaction between the soma and the dendritic compartment.
Therefore, we evaluate the firing rate and the dendritic prediction at equal
time points to calculate the error signal at that time point. Due to the
synaptic delay :math:`d`, the synapse combines a delayed version of the error
signal with the presynaptic spike train (:math:`S_{pre}`), see :ref:`panel c)
<fig-multicompartment>`.

.. _fig-multicompartment::

.. figure:: ../static/img/multicompartment.png
   :width: 75 %

   a) Two point neurons (red circles *pre* and *post*) connected via a synapse.
   In NEST, the delay is entirely on the postsynaptic side, and in the case of point
   neurons, it is interpreted as the dendritic delay. b) Two two-compartment
   neuron models composed of a somatic (green) and a dendritic (blue)
   compartment. The soma of the presynaptic neuron is connected to the dendrite
   of the postsynaptic neuron. The synaptic delay is located behind the synapse
   and before the dendrite. c) Time trace of the State variables that enter the
   Urbanczik-Senn rule. Due to the synaptic delay :math:`d`, the presynaptic
   spike train (top) is combined with a delayed version of the postsynaptic
   quantities; the dendritic membrane potential (middle) and the somatic spike
   train (bottom).


Parameters
++++++++++

The following parameters can be set in the status dictionary. Parameters
for each compartment are collected in a sub-dictionary; these sub-dictionaries
are called "soma" and "dendritic", respectively. In the list below,
these parameters are marked with an asterisk.

============   =====   =====================================================
 V_m*           mV      Membrane potential
 E_L*           mV      Leak reversal potential
 C_m*           pF      Capacity of the membrane
 E_ex*          mV      Excitatory reversal potential
 E_in*          mV      Inhibitory reversal potential
 g_L*           nS      Leak conductance
 tau_syn_ex*    ms      Rise time of the excitatory synaptic alpha function
 tau_syn_in*    ms      Rise time of the inhibitory synaptic alpha function
 I_e*           pA      Constant input current
 g_sp           nS      Coupling between soma and dendrite
 g_ps           nS      Coupling between dendrite and soma
 t_ref          ms      Duration of refractory period
============   =====   =====================================================

See :doc:`../auto_examples/urbanczik_synapse_example` to learn more.

Remarks:

The neuron model uses standard units of NEST instead of the unitless quantities
used in [1]_.

.. note::
   All parameters that occur for both compartments are stored as C arrays, with
   index 0 being soma.

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

.. [1] R. Urbanczik, W. Senn (2014). Learning by the Dendritic Prediction of
       Somatic Spiking. Neuron, 81, 521 - 528.

See also
++++++++

urbanczik_synapse

EndUserDocs */

class pp_cond_exp_mc_urbanczik : public UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >
{

  // Boilerplate function declarations --------------------------------

public:
  pp_cond_exp_mc_urbanczik();
  pp_cond_exp_mc_urbanczik( const pp_cond_exp_mc_urbanczik& );
  ~pp_cond_exp_mc_urbanczik();

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
  void init_buffers_();
  void calibrate();
  void update( Time const&, const long, const long );

  // Enumerations and constants specifying structure and properties ----

  //! Compartments, NCOMP is number
  enum Compartments_
  {
    SOMA = 0,
    DEND,
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
    DEND_EXC,
    DEND_INH,
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
    I_DEND,
    SUP_CURR_RECEPTOR
  };

  static const size_t NUM_CURR_RECEPTORS = SUP_CURR_RECEPTOR - MIN_CURR_RECEPTOR;

  // Friends --------------------------------------------------------

  friend int pp_cond_exp_mc_urbanczik_dynamics( double, const double*, double*, void* );

  friend class RecordablesMap< pp_cond_exp_mc_urbanczik >;
  friend class UniversalDataLogger< pp_cond_exp_mc_urbanczik >;


  // Parameters ------------------------------------------------------

  /**
   * Independent parameters of the model.
   * These parameters must be passed to the iteration function that
   * is passed to the GSL ODE solvers. Since the iteration function
   * is a C++ function with C linkage, the parameters can be stored
   * in a C++ struct with member functions, as long as we just pass
   * it by void* from C++ to C++ function. The struct must be public,
   * though, since the iteration function is a function with C-linkage,
   * whence it cannot be a member function of pp_cond_exp_mc_urbanczik.
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
    double t_ref;         //!< Refractory period in ms
    double E_ex[ NCOMP ]; //!< Excitatory reversal Potential in mV
    double E_in[ NCOMP ]; //!< Inhibitory reversal Potential in mV
    double I_e[ NCOMP ];  //!< Constant Current in pA

    pp_cond_exp_mc_urbanczik_parameters urbanczik_params;

    /** Dead time in ms. */
    double dead_time_;

    Parameters_();                                //!< Sets default parameter values
    Parameters_( const Parameters_& );            //!< needed to copy C-arrays
    Parameters_& operator=( const Parameters_& ); //!< needed to copy C-arrays

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };


  // State variables  ------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor required because of C-style array.
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
      G_EXC,
      G_INH,
      I_EXC, // in the paper it is I_dnd which accounts for both excitation and inhibition
      I_INH,
      STATE_VEC_COMPS
    };

    //! total size of state vector
    static const size_t STATE_VEC_SIZE = STATE_VEC_COMPS * NCOMP;

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    int r_; //!< number of refractory steps remaining

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
      assert( comp * STATE_VEC_COMPS + elem < STATE_VEC_SIZE );
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
    Buffers_( pp_cond_exp_mc_urbanczik& ); //!<Sets buffer pointers to 0
    //! Sets buffer pointers to 0
    Buffers_( const Buffers_&, pp_cond_exp_mc_urbanczik& );

    //! Logger for all analog data
    UniversalDataLogger< pp_cond_exp_mc_urbanczik > logger_;

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
    double step_;            //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL

    /**
     * Input currents injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_[ NCOMP ]; //!< External Stimulus in pA
  };

  // Internal variables ---------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    int RefractoryCounts_;

    double h_;                          //!< simulation time step in ms
    RngPtr rng_;                        //!< random number generator of my own thread
    poisson_distribution poisson_dist_; //!< poisson distribution
  };

  // Access functions for UniversalDataLogger -------------------------------

  /**
   * Read out state vector elements, used by UniversalDataLogger
   * First template argument is component "name", second compartment "name".
   */
  template < State_::StateVecElems_ elem, Compartments_ comp >
  double
  get_y_elem_() const
  {
    return S_.y_[ S_.idx( comp, elem ) ];
  }

  //! Read out number of refractory steps, used by UniversalDataLogger
  double
  get_r_() const
  {
    return Time::get_resolution().get_ms() * S_.r_;
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
  static RecordablesMap< pp_cond_exp_mc_urbanczik > recordablesMap_;
};


// Inline functions of pp_cond_exp_mc_urbanczik_parameters
inline double
pp_cond_exp_mc_urbanczik_parameters::phi( double u )
{
  return phi_max / ( 1.0 + rate_slope * exp( beta * ( theta - u ) ) );
}

inline double
pp_cond_exp_mc_urbanczik_parameters::h( double u )
{
  return 15.0 * beta / ( 1.0 + ( 1.0 / rate_slope ) * exp( -beta * ( theta - u ) ) );
}


// Inline functions of pp_cond_exp_mc_urbanczik
inline port
pp_cond_exp_mc_urbanczik::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
pp_cond_exp_mc_urbanczik::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type < MIN_SPIKE_RECEPTOR || receptor_type >= SUP_SPIKE_RECEPTOR )
  {
    if ( receptor_type < 0 || receptor_type >= SUP_CURR_RECEPTOR )
    {
      throw UnknownReceptorType( receptor_type, get_name() );
    }
    else
    {
      throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
    }
  }
  return receptor_type - MIN_SPIKE_RECEPTOR;
}

inline port
pp_cond_exp_mc_urbanczik::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type < MIN_CURR_RECEPTOR || receptor_type >= SUP_CURR_RECEPTOR )
  {
    if ( receptor_type >= 0 && receptor_type < MIN_CURR_RECEPTOR )
    {
      throw IncompatibleReceptorType( receptor_type, get_name(), "CurrentEvent" );
    }
    else
    {
      throw UnknownReceptorType( receptor_type, get_name() );
    }
  }
  return receptor_type - MIN_CURR_RECEPTOR;
}

inline port
pp_cond_exp_mc_urbanczik::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    if ( receptor_type < 0 || receptor_type >= SUP_CURR_RECEPTOR )
    {
      throw UnknownReceptorType( receptor_type, get_name() );
    }
    else
    {
      throw IncompatibleReceptorType( receptor_type, get_name(), "DataLoggingRequest" );
    }
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
pp_cond_exp_mc_urbanczik::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  /**
   * @todo dictionary construction should be done only once for
   * static member in default c'tor, but this leads to
   * a seg fault on exit, see #328
   */
  DictionaryDatum receptor_dict_ = new Dictionary();
  ( *receptor_dict_ )[ names::soma_exc ] = SOMA_EXC;
  ( *receptor_dict_ )[ names::soma_inh ] = SOMA_INH;
  ( *receptor_dict_ )[ names::soma_curr ] = I_SOMA;

  ( *receptor_dict_ )[ names::dendritic_exc ] = DEND_EXC;
  ( *receptor_dict_ )[ names::dendritic_inh ] = DEND_INH;
  ( *receptor_dict_ )[ names::dendritic_curr ] = I_DEND;

  ( *d )[ names::receptor_types ] = receptor_dict_;
}

inline void
pp_cond_exp_mc_urbanczik::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, ptmp );   // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  UrbanczikArchivingNode< pp_cond_exp_mc_urbanczik_parameters >::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace


#endif // HAVE_GSL
#endif // PP_COND_EXP_MC_URBANCZIK_H
