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
#include "universal_data_logger_impl.h"

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

// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, conductance-based, hard threshold

Short description
+++++++++++++++++

Leaky integrate-and-fire neuron model with exponentially-shaped synaptic conductances


Description
+++++++++++

``iaf_cond_exp`` is a leaky integrate-and-fire neuron model with

* a hard threshold,
* a fixed refractory period,
* no adaptation mechanisms,
* exponentially-shaped synaptic conductances according to [1]_, normalized such that an event of weight 1.0 results in a peak conductance of 1 nS.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The membrane potential evolves according to

.. math::

   \frac{dV_\text{m}}{dt} = \frac{ -g_{\text{L}} (V_{\text{m}} - E_{\text{L}}) - I_{\text{syn}} + I_\text{e} } {C_{\text{m}}}

where the synaptic input current :math:`I_{\text{syn}}(t)` is discussed below and :math:`I_\text{e}` is
a constant input current set as a model parameter.

A spike is emitted at time step :math:`t^*=t_{k+1}` if

.. math::

   V_\text{m}(t_k) < V_{th} \quad\text{and}\quad V_\text{m}(t_{k+1})\geq V_\text{th} \;.

Subsequently,

.. math::

   V_\text{m}(t) = V_{\text{reset}} \quad\text{for}\quad t^* \leq t < t^* + t_{\text{ref}} \;,

that is, the membrane potential is clamped to :math:`V_{\text{reset}}` during the refractory period.

Synaptic input
..............

The synaptic input current has an excitatory and an inhibitory component

.. math::

   I_{\text{syn}}(t) = I_{\text{syn, ex}}(t) + I_{\text{syn, in}}(t)

where

.. math::

   I_{\text{syn, X}}(t) = (V_{\text{m}}(t) - E_{\text{syn, X}}) \sum_{j}  \sum_k g_{\text{j, X}}(t-t_j^k-d_j) \;,

where :math:`j` indexes either excitatory (:math:`\text{X} = \text{ex}`)
or inhibitory (:math:`\text{X} = \text{in}`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, and :math:`d_j`
is the delay from neuron :math:`j`.

The individual synaptic conductances are given by

.. math::

   g_{\text{j, X}}(t) = w_{\text{j}} \cdot e^{-\frac{t}{\tau_{\text{syn, X}}}} \Theta(t)

where :math:`\Theta(x)` is the Heaviside step function. The conductances are normalized to unit maximum, that is,

.. math::

   g_{\text{j, X}}(t= 0) = w_{\text{j}} \;,

where :math:`w` is a weight (excitatory if :math:`w > 0` or inhibitory if :math:`w < 0`).

Parameters
++++++++++

=============== ================== =============================== ========================================================================
**Parameter**   **Default**        **Math equivalent**             **Description**
=============== ================== =============================== ========================================================================
``E_L``         -70 mV             :math:`E_\text{L}`              Leak reversal potential
``C_m``         250 pF             :math:`C_{\text{m}}`            Capacity of the membrane
``t_ref``       2 ms               :math:`t_{\text{ref}}`          Duration of refractory period
``V_th``        -55 mV             :math:`V_{\text{th}}`           Spike threshold
``V_reset``     -70 mV             :math:`V_{\text{reset}}`        Reset potential of the membrane
``E_ex``        0 mV               :math:`E_\text{ex}`             Excitatory reversal potential
``E_in``        -85 mV             :math:`E_\text{in}`             Inhibitory reversal potential
``g_L``         -16.6667 nS        :math:`g_\text{L}`              Leak conductance
``tau_syn_ex``  0.2 ms             :math:`\tau_{\text{syn, ex}}`   Exponential decay time constant of excitatory synaptic conductance kernel
``tau_syn_in``  2.0 ms             :math:`\tau_{\text{syn, in}}`   Exponential decay time constant of inhibitory synaptic conductance kernel
``I_e``         0 pA               :math:`I_\text{e}`              Constant input current
=============== ================== =============================== ========================================================================

The following state variables evolve during simulation and are available either as neuron properties or as recordables.

================== ================= ========================== =================================
**State variable** **Initial value** **Math equivalent**        **Description**
================== ================= ========================== =================================
``V_m``            -70 mV            :math:`V_{\text{m}}`       Membrane potential
``g_ex``           0 nS              :math:`g_{\text{ex}}`      Excitatory synaptic conductance
``g_in``           0 nS              :math:`g_{\text{in}}`      Inhibitory synaptic conductance
================== ================= ========================== =================================

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

.. [1] Meffin H, Burkitt AN, Grayden DB (2004). An analytical
       model for the large, fluctuating synaptic conductance state typical of
       neocortical neurons in vivo. Journal of Computational Neuroscience,
       16:159-175.
       DOI: https://doi.org/10.1023/B:JCNS.0000014108.03012.81

See also
++++++++

iaf_psc_delta, iaf_psc_alpha, iaf_psc_exp, iaf_cond_alpha, iaf_cond_beta

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_cond_exp

EndUserDocs*/
// clang-format on

void register_iaf_cond_exp( const std::string& name );

class iaf_cond_exp : public ArchivingNode
{

public:
  iaf_cond_exp();
  iaf_cond_exp( const iaf_cond_exp& );
  ~iaf_cond_exp() override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;
  void update( Time const&, const long, const long ) override;

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
    double tau_synE; //!< Time constant for excitatory synaptic kernel in ms
    double tau_synI; //!< Time constant for inhibitory synaptic kernel in ms
    double I_e;      //!< Constant Current in pA

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor required because of C-style array.
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
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_cond_exp& );                  //!< Sets buffer pointers to 0
    Buffers_( const Buffers_&, iaf_cond_exp& ); //!< Sets buffer pointers to 0

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

    // Since IntegrationStep_ is initialized with step_, and the resolution
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


inline size_t
nest::iaf_cond_exp::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_cond_exp::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_cond_exp::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_cond_exp::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
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
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_cond_exp::set_status( const DictionaryDatum& d )
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

template <>
void RecordablesMap< iaf_cond_exp >::create();

} // namespace

#endif // HAVE_GSL
#endif // IAF_COND_EXP_H
