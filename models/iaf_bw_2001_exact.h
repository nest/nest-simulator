/*
 *  iaf_bw_2001_exact.h
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

#ifndef IAF_BW_2001_EXACT
#define IAF_BW_2001_EXACT

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C includes:
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
 **/
extern "C" inline int iaf_bw_2001_exact_dynamics( double, const double y[], double f[], void* pnode );

// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, conductance-based

Short description
+++++++++++++++++

Leaky integrate-and-fire-neuron model with conductance-based synapses and additional NMDA receptors.

Description
+++++++++++

``iaf_bw_2001_exact`` is a leaky integrate-and-fire neuron model with

* an exact implementation of the neuron model described in [1]_.
* exponential conductance-based AMPA and GABA-synapses
* NMDA synapses with slow nonlinear dynamics
* a fixed refractory period
* no adaptation mechanisms

Neuron and synaptic dynamics
..................................................

The membrane potential and synaptic variables evolve according to

.. math::

    C_\mathrm{m} \frac{dV(t)}{dt} &= -g_\mathrm{L} (V(t) - V_\mathrm{L}) - I_\mathrm{syn} (t) \\[3ex]
    I_\mathrm{syn}(t) &= I_\mathrm{AMPA}(t) + I_\mathrm{NMDA}(t) + I_\mathrm{GABA}(t) (t) \\[3ex]
    I_\mathrm{AMPA} &= (V(t) - V_E)\sum_{j \in \Gamma_\mathrm{ex}}^{N_E}w_jS_{j,\mathrm{AMPA}}(t) \\[3ex]
    I_\mathrm{NMDA} &= \frac{(V(t) - V_E)}{1+[\mathrm{Mg^{2+}}]\mathrm{exp}(-0.062V(t))/3.57}\sum_{j \in
    \Gamma_\mathrm{ex}}^{N_E}w_jS_{j,\mathrm{NMDA}}(t) \\[3ex]
    I_\mathrm{GABA} &= (V(t) - V_I)\sum_{j \in \Gamma_\mathrm{in}}^{N_E}w_jS_{j,\mathrm{GABA}}(t) \\[5ex]
    \frac{dS_{j,\mathrm{AMPA}}}{dt} &=-\frac{j,S_{\mathrm{AMPA}}}{\tau_\mathrm{AMPA}}+\sum_{k \in \Delta_j} \delta (t - t_j^k) \\[3ex]
    \frac{dS_{j,\mathrm{GABA}}}{dt} &= -\frac{S_{j,\mathrm{GABA}}}{\tau_\mathrm{GABA}} + \sum_{k \in \Delta_j} \delta (t - t_j^k) \\[3ex]
    \frac{dS_{j,\mathrm{NMDA}}}{dt} &= -\frac{S_{j,\mathrm{NMDA}}}{\tau_\mathrm{NMDA,decay}}+ \alpha x_j (1 - S_{j,\mathrm{NMDA}})\\[3ex]
    \frac{dx_j}{dt} &= -\frac{x_j}{\tau_\mathrm{NMDA,rise}} + \sum_{k \in \Delta_j} \delta (t - t_j^k)

where :math:`\Gamma_\mathrm{ex}` and :math:`\Gamma_\mathrm{in}` are index sets for presynaptic excitatory and inhibitory
neurons respectively, and :math:`\Delta_j` is an index set for the spike times of neuron :math:`j`.

Since :math:`S_{j,\mathrm{AMPA}}` and :math:`S_{j,\mathrm{GABA}}` are piecewise exponential functions, the sums are also
a piecewise exponential function, and can be stored in a single synaptic variable each, :math:`S_{\mathrm{AMPA}}` and
:math:`S_{\mathrm{GABA}}` respectively. The sum over :math:`S_{j,\mathrm{NMDA}}` does not have a simple expression, and
cannot be simplified. Therefore, for each synapse, we need to integrate separate state variables, which makes the model
slow.

The specification of this model differs slightly from the one in [1]_. The parameters :math:`g_\mathrm{AMPA}`,
:math:`g_\mathrm{GABA}`, and :math:`g_\mathrm{NMDA}` have been absorbed into the respective synaptic weights.
Additionally, the synapses from the external population is not separated from the recurrent AMPA-synapses.
This model is slow to simulate when there are many neurons with NMDA-synapses, since each post-synaptic neuron simulates each pre-synaptic connection explicitly. The model :doc:`iaf_bw_2001 </models/iaf_bw_2001>` is an approximation to this model which is significantly faster.

See also [2]_, [3]_

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=================== ================== ================================= ========================================================================
**Parameter**       **Default**        **Math equivalent**               **Description**
=================== ================== ================================= ========================================================================
``E_L``             -70.0 mV           :math:`E_\mathrm{L}`              Leak reversal potential
``E_ex``              0.0 mV           :math:`E_\mathrm{ex}`             Excitatory reversal potential
``E_in``            -70.0 mV           :math:`E_\mathrm{in}`             Inhibitory reversal potential
``V_th``            -55.0 mV           :math:`V_\mathrm{th}`             Spike threshold
``V_reset``         -60.0 mV           :math:`V_\mathrm{reset}`          Reset potential of the membrane
``C_m``             250.0 pF           :math:`C_\mathrm{m}`              Capacitance of the membrane
``g_L``              25.0 nS           :math:`g_\mathrm{L}`              Leak conductance
``t_ref``             2.0 ms           :math:`t_\mathrm{ref}`            Duration of refractory period
``tau_AMPA``          2.0 ms           :math:`\tau_\mathrm{AMPA}`        Time constant of AMPA synapse
``tau_GABA``          5.0 ms           :math:`\tau_\mathrm{GABA}`        Time constant of GABA synapse
``tau_rise_NMDA``     2.0 ms           :math:`\tau_\mathrm{NMDA,rise}`   Rise time constant of NMDA synapse
``tau_decay_NMDA``  100.0 ms           :math:`\tau_\mathrm{NMDA,decay}`  Decay time constant of NMDA synapse
``alpha``             0.5 ms^{-1}      :math:`\alpha`                    Rise-time coupling strength for NMDA synapse
``conc_Mg2``          1.0 mM           :math:`[\mathrm{Mg}^+]`           Extracellular magnesium concentration
``gsl_error_tol``    1e-3                                                Error tolerance for GSL RKF45-solver
=================== ================== ================================= ========================================================================

The following state variables evolve during simulation and are available either as neuron properties or as recordables.

================== ================= ========================== =================================
**State variable** **Initial value** **Math equivalent**        **Description**
================== ================= ========================== =================================
``V_m``            -70 mV            :math:`V_{\mathrm{m}}`     Membrane potential
``s_AMPA``           0               :math:`s_\mathrm{AMPA}`    AMPA gating variable
``s_GABA``           0               :math:`s_\mathrm{GABA}`    GABA gating variable
``s_NMDA``           0               :math:`s_\mathrm{NMDA}`    NMDA gating variable
``I_NMDA``           0 pA            :math:`I_\mathrm{NMDA}`    NMDA current
``I_AMPA``           0 pA            :math:`I_\mathrm{AMPA}`    AMPA current
``I_GABA``           0 pA            :math:`I_\mathrm{GABA}`    GABA current
================== ================= ========================== =================================

.. note::
   It is possible to set values for :math:`V_\mathrm{m}`, :math:`S_\mathrm{AMPA}` and :math:`S_\mathrm{GABA}` when creating the model, while the different :math:`s_{j,\mathrm{NMDA}}` (`j` represents presynaptic neuron `j`) can not be set by the user.

.. note::
   :math:`g_{\mathrm{\{\{rec,AMPA\}, \{ext,AMPA\}, GABA, NMBA}\}}` from [1]_ is built into the weights in this NEST model, so these variables are set by changing the weights.

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

.. [1] Wang, X.-J. (1999). Synaptic Basis of Cortical Persistent Activity: The Importance of NMDA Receptors to Working Memory. Journal of Neuroscience, 19(21), 9587–9603. https://doi.org/10.1523/JNEUROSCI.19-21-09587.1999
.. [2] Brunel, N., & Wang, X.-J. (2001). Effects of Neuromodulation in a Cortical Network Model of Object Working Memory Dominated by Recurrent Inhibition. Journal of Computational Neuroscience, 11(1), 63–85. https://doi.org/10.1023/A:1011204814320
.. [3] Wang, X. J. (2002). Probabilistic decision making by slow reverberation in
       cortical circuits. Neuron, 36(5), 955-968. https://doi.org/10.1016/S0896-6273(02)01092-9

See also
++++++++

iaf_bw_2001

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_bw_2001_exact

EndUserDocs */
// clang-format on

void register_iaf_bw_2001_exact( const std::string& name );

class iaf_bw_2001_exact : public ArchivingNode
{
public:
  iaf_bw_2001_exact();
  iaf_bw_2001_exact( const iaf_bw_2001_exact& );
  ~iaf_bw_2001_exact() override;

  /*
   * Import all overloaded virtual functions that we
   * override in this class.  For background information,
   * see http://www.gotw.ca/gotw/005.htm.
   */

  using Node::handle;
  using Node::handles_test_event;

  /**
   * Used to validate that we can send SpikeEvent to desired target:port.
   **/
  size_t send_test_event( Node& target, size_t receptor_type, synindex, bool ) override;

  void handle( SpikeEvent& ) override;         //!< accept spikes
  void handle( CurrentEvent& e ) override;     //!< accept current
  void handle( DataLoggingRequest& ) override; //!< allow recording with multimeter

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  /* -------------------------------------------------------------------------
   * Functions for getting/setting parameters and state values.
   * ------------------------------------------------------------------------- */

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_state_() override;
  void pre_run_hook() override;
  void init_buffers_() override;
  void update( Time const&, const long, const long ) override;

  /**
   * Synapse types to connect to
   **/
  enum SynapseTypes
  {
    INF_SPIKE_RECEPTOR = 0,
    AMPA,
    GABA,
    NMDA,
    SUP_SPIKE_RECEPTOR
  };

  // make dynamics function quasi-member
  friend int iaf_bw_2001_exact_dynamics( double, const double y[], double f[], void* pnode );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_bw_2001_exact >;
  friend class UniversalDataLogger< iaf_bw_2001_exact >;

  struct Parameters_
  {
    double E_L;            //!< Resting Potential in mV
    double E_ex;           //!< Excitatory reversal Potential in mV
    double E_in;           //!< Inhibitory reversal Potential in mV
    double V_th;           //!< Threshold Potential in mV
    double V_reset;        //!< Reset Potential in mV
    double C_m;            //!< Membrane Capacitance in pF
    double g_L;            //!< Leak Conductance in nS
    double t_ref;          //!< Refractory period in ms
    double tau_AMPA;       //!< Synaptic Time Constant AMPA Synapse in ms
    double tau_GABA;       //!< Synaptic Time Constant GABA Synapse in ms
    double tau_rise_NMDA;  //!< Synaptic Rise Time Constant NMDA Synapse in ms
    double tau_decay_NMDA; //!< Synaptic Decay Time Constant NMDA Synapse in ms
    double alpha;          //!< Scaling factor for NMDA synapse in 1/ms
    double conc_Mg2;       //!< Extracellular Magnesium Concentration in mM

    double gsl_error_tol; //!< GSL Error Tolerance

    /**
     * Initialize parameters to their default values.
     **/
    Parameters_();

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

public:
  // State variables class --------------------------------------------

  /**
   * State variables of the model.
   *
   * State variables consist of the state vector for the subthreshold
   * dynamics and the refractory count. The state vector must be a
   * C-style array to be compatible with GSL ODE solvers.
   *
   * @note Copy constructor is required because of the C-style array.
   */
  struct State_
  {
    /**
     * Symbolic indices to the elements of the state vector y
     * (x_NMDA_1, s_NMDA_1), (x_NMDA_2, s_NMDA_2), (x_NMDA_3, s_NMDA_3), ..., (x_NMDA_j, s_NMDA_j)
     */
    enum StateVecElems
    {
      V_m = 0,
      s_AMPA,
      s_GABA,
      s_NMDA_base,
    };

    size_t state_vec_size;

    double* ode_state_; //!< state vector, must be C-array for GSL solver
    long num_ports_;    //!< Number of ports
    int r_;             //!< number of refractory steps remaining

    double s_NMDA_sum; // For recording NMDA gating variables
    double I_NMDA_;    // For recording NMDA currents
    double I_AMPA_;    // For recording NMDA currents
    double I_GABA_;    // For recording NMDA currents

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

private:
  // Buffers class --------------------------------------------------------

  /**
   * Buffers of the model.
   * Buffers are on par with state variables in terms of persistence,
   * i.e., initialized only upon first Simulate call after ResetKernel,
   * but its implementation details hidden from the user.
   */
  struct Buffers_
  {
    Buffers_( iaf_bw_2001_exact& );
    Buffers_( const Buffers_&, iaf_bw_2001_exact& );

    /**
     * Logger for all analog data
     **/
    UniversalDataLogger< iaf_bw_2001_exact > logger_;

    // -----------------------------------------------------------------------
    //   Buffers and sums of incoming spikes and currents per timestep
    // -----------------------------------------------------------------------
    std::vector< RingBuffer > spikes_;
    RingBuffer currents_;

    /**
     * Vector for weights
     */
    std::vector< double > weights_;

    // -----------------------------------------------------------------------
    //   GSL ODE solver data structures
    // -----------------------------------------------------------------------

    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    /*
     * integration_step_ should be reset with the neuron on ResetNetwork,
     * but remain unchanged during calibration. Since it is initialized with
     * step_, and the resolution cannot change after nodes have been created,
     * it is safe to place both here.
     */
    double step_;             //!< step size in ms
    double integration_step_; //!< current integration time step, updated by GSL

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;
  };

  // Variables class -------------------------------------------------------

  /**
   * Internal variables of the model.
   * Variables are re-initialized upon each call to Simulate.
   */
  struct Variables_
  {
    //! refractory time in steps
    long RefractoryCounts;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_ode_state_elem_() const
  {
    return S_.ode_state_[ elem ];
  }

  //! Get NMDA current / gating variable from state, used by UniversalDataLogger
  double
  get_s_NMDA_() const
  {
    return S_.s_NMDA_sum;
  }
  double
  get_I_NMDA_() const
  {
    return S_.I_NMDA_;
  }
  double
  get_I_AMPA_() const
  {
    return S_.I_AMPA_;
  }
  double
  get_I_GABA_() const
  {
    return S_.I_GABA_;
  }


  // Data members -----------------------------------------------------------

  // keep the order of these lines, seems to give best performance
  Parameters_ P_; //!< Free parameters.
  State_ S_;      //!< Dynamic state.
  Variables_ V_;  //!< Internal Variables
  Buffers_ B_;    //!< Buffers.

  //! Mapping of recordables names to access functions
  static RecordablesMap< iaf_bw_2001_exact > recordablesMap_;


}; /* neuron iaf_bw_2001_exact */

inline size_t
iaf_bw_2001_exact::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_bw_2001_exact::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( not( INF_SPIKE_RECEPTOR < receptor_type and receptor_type < SUP_SPIKE_RECEPTOR ) )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
    return 0;
  }
  else
  {
    if ( receptor_type == SynapseTypes::NMDA )
    {
      // after the buffers are initialized, new synapses cannot be added since the buffers would have
      // to be expanded
      if ( B_.e_ != nullptr )
      {
        throw IllegalConnection(
          "NMDA connections to this model can only be made before the first call to nest.Simulate()" );
      }
      // give each NMDA synapse a unique rport, starting from 3 (num_ports_ is initialized to 3)
      ++S_.num_ports_;
      return S_.num_ports_;
    }
    else
    {
      return receptor_type;
    }
  }
}

inline size_t
iaf_bw_2001_exact::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_bw_2001_exact::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  /*
   * You should usually not change the code in this function.
   * It confirms to the connection management system that we are able
   * to handle @c DataLoggingRequest on port 0.
   * The function also tells the built-in UniversalDataLogger that this node
   * is recorded from and that it thus needs to collect data during simulation.
   */
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_bw_2001_exact::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  DictionaryDatum receptor_type = new Dictionary();

  ( *receptor_type )[ names::AMPA ] = AMPA;
  ( *receptor_type )[ names::GABA ] = GABA;
  ( *receptor_type )[ names::NMDA ] = NMDA;

  ( *d )[ names::receptor_types ] = receptor_type;

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_bw_2001_exact::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

  /*
   * We now know that (ptmp, stmp) are consistent. We do not
   * write them back to (P_, S_) before we are also sure that
   * the properties to be set in the parent class are internally
   * consistent.
   */
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
};
} // namespace

#endif // HAVE_GSL
#endif // IAF_BW_2001
