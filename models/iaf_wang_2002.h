/*
 *  iaf_wang_2002.h
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

#ifndef IAF_WANG_2002_H
#define IAF_WANG_2002_H

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
 */
extern "C" inline int iaf_wang_2002_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: neuron, integrate-and-fire, conductance-based

DOCUMENTATION WILL BE UPDATED

Short description
+++++++++++++++++

Leaky integrate-and-fire-neuron model with dynamic NMDA receptors.

Description
+++++++++++

This model implements a simplified version of the neuron model described in [1]_.

It contains AMPA, GABA and NMDA synapses

The equations for the synaptic currents from AMPA and GABA receptors are given by
the following equations

.. math::
    I_{\mathrm{AMPA}} = g_{\mathrm{AMPA}}(V(t) - E_{\mathrm{ex}} \sum_{j=1}^{C_E} w_j s_j^{\mathrm{AMPA}}
    \frac{d}{dt}s^{\mathrm{AMPA}}_j = -\frac{s_j}{\tau_{\mathrm{AMPA}}}

.. math::
   \frac{ ds_j^{NMDA}(t) }{ dt } = - \frac{ s_j^{NMDA}(t) }{ \tau_{NMDA,decay} } + \alpha x_j(t)(1 - s_j^{NMDA}(t)) \\
   \frac{ dx_j(t) }{ dt } =- \frac{ x_j(t) }{ \tau_{NMDA,rise} } + \sum_k \delta(t - t_j^k).

The synaptic current of NMDA is given by

.. math::
   I_{NMDA}(t) = \frac{ V(t) - E_{ex} }{ 1 + [Mg^{2+}]exp(-0.062V(t))/3.57 }\sum_{j=1}w_jg_j^{NMDA},

where `w_j` is the weight of connection with presynaptic neuron `j`.


Parameters
++++++++++

The following parameters can be set in the status dictionary.

=============== ======= ===========================================================
 E_L            mV      Resting potential
 E_ex           mV      Excitatory reversal potential
 E_in           mV      Inhibitory reversal potential
 V_th           mV      Threshold potential
 V_reset        mV      Reset potential
 C_m            pF      Membrane capacitance
 g_L            nS      Leak conductance
 g_AMPA         nS      Peak recurrent AMPA conductance 
 g_NMDA         nS      Peak recurrent NMDA conductance 
 g_GABA         nS      Peak recurrent GABA conductance 
 g_L            nS      Leak conductance
 t_ref          ms      Refractory period
 tau_AMPA       ms      Synaptic time constant for AMPA synapse
 tau_GABA       ms      Synaptic time constant for GABA synapse
 tau_decay_NMDA ms      Synaptic decay time constant for NMDA synapse
 alpha          1/ms    Scaling factor for NMDA synapse
 conc_Mg2       mM      Extracellular magnesium concentration
 gsl_error_tol  -       GSL error tolerance
=============== ======= ===========================================================

Recordables
+++++++++++

The following values can be recorded.

=========== ===========================================================
 V_m         Membrane potential
 s_AMPA      AMPA gate
 s_GABA      GABA gate
 NMDA_sum    sum of NMDA over all presynaptic neurons j
=========== ===========================================================

.. note::
   It is possible to set values for `V_m`, `g_AMPA` and `g_GABA` when creating the model, while the
   different `g_NMDA_j` (`j` represents presynaptic neuron `j`) can not be set by the user.

.. note::
   The variable `g_AMPA` and `g_GABA` in the NEST implementation does not correspond to `g_{recAMPA, extAMPA, GABA}`
   in [1]_. `g_{recAMPA, extAMPA, GABA, NMBA}` from [1]_ is built into the weights in this NEST model, so setting the
   variables is thus done by changing the weights.

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

.. [1] Wang, X. J. (2002). Probabilistic decision making by slow reverberation in
       cortical circuits. Neuron, 36(5), 955-968.
       DOI: https://doi.org/10.1016/S0896-6273(02)01092-9

See also
++++++++

iaf_cond_alpha, ht_neuron

EndUserDocs */

void register_iaf_wang_2002( const std::string& name );

class iaf_wang_2002 : public ArchivingNode
{
public:
  iaf_wang_2002();
  iaf_wang_2002( const iaf_wang_2002& );
  ~iaf_wang_2002() override;

  /**
   * Import all overloaded virtual functions that we
   * override in this class.  For background information,
   * see http://www.gotw.ca/gotw/005.htm.
   */

  using Node::handles_test_event;
  using Node::handle;

  //! Used to validate that we can send SpikeEvent to desired target:port.
  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;         //!< accept spikes
  void handle( CurrentEvent& ) override;     //!< accept current
  void handle( DataLoggingRequest& ) override; //!< allow recording with multimeter

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  /* -------------------------------------------------------------------------
   * Functions for getting/setting parameters and state values.
   * ------------------------------------------------------------------------- */

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  bool
    is_off_grid() const override
    {
      return true;
    }

private:
  void init_state_() override;
  void pre_run_hook() override;
  void init_buffers_() override;
  void calibrate();
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
  friend int iaf_wang_2002_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_wang_2002 >;
  friend class UniversalDataLogger< iaf_wang_2002 >;

  struct Parameters_
  {
    double E_L;            //!< Resting Potential in mV
    double E_ex;           //!< Excitatory reversal Potential in mV
    double E_in;           //!< Inhibitory reversal Potential in mV
    double V_th;           //!< Threshold Potential in mV
    double V_reset;        //!< Reset Potential in mV
    double C_m;            //!< Membrane Capacitance in pF
    double g_L;            //!< Leak Conductance in nS
//     double g_GABA;         //!< Peak conductance GABA
//     double g_NMDA;         //!< Peak conductance NMDA
//     double g_AMPA;         //!< Peak conductance AMPA
//     double g_AMPA_ext;         //!< Peak conductance AMPA
    double t_ref;          //!< Refractory period in ms
    double tau_AMPA;       //!< Synaptic Time Constant AMPA Synapse in ms
    double tau_GABA;       //!< Synaptic Time Constant GABA Synapse in ms
    double tau_decay_NMDA; //!< Synaptic Decay Time Constant NMDA Synapse in ms
    double alpha;          //!< Scaling factor for NMDA synapse in 1/ms
    double conc_Mg2;       //!< Extracellular Magnesium Concentration in mM

    double gsl_error_tol; //!< GSL Error Tolerance

    //! Initialize parameters to their default values.
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
    //! Symbolic indices to the elements of the state vector y
    enum StateVecElems
    {
      V_m = 0,
      s_AMPA,
      s_GABA,
      s_NMDA,
      STATE_VEC_SIZE
    };

    double y_[ STATE_VEC_SIZE ]; //!< state vector, must be C-array for GSL solver
    double s_NMDA_pre;  // for determining (unweighted) alpha * (1 - s_NMDA) term on
                        // pre-synaptic side
    int r_;             //!< number of refractory steps remaining

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
    Buffers_( iaf_wang_2002& );
    Buffers_( const Buffers_&, iaf_wang_2002& );

    //! Logger for all analog data
    UniversalDataLogger< iaf_wang_2002 > logger_;

    // -----------------------------------------------------------------------
    //   Buffers and sums of incoming spikes and currents per timestep
    // -----------------------------------------------------------------------
    std::vector< RingBuffer > spikes_;
//     RingBuffer spike_AMPA_;
//     RingBuffer spike_GABA_;
//     RingBuffer spike_NMDA_;
    RingBuffer currents_;

    // -----------------------------------------------------------------------
    //   GSL ODE solver data structures
    // -----------------------------------------------------------------------

    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    /**
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
    long RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_ode_state_elem_() const
  {
    return S_.y_[ elem ];
  }

  // Data members -----------------------------------------------------------

  // keep the order of these lines, seems to give best performance
  Parameters_ P_; //!< Free parameters.
  State_ S_;      //!< Dynamic state.
  Variables_ V_;  //!< Internal Variables
  Buffers_ B_;    //!< Buffers.

  //! Mapping of recordables names to access functions
  static RecordablesMap< iaf_wang_2002 > recordablesMap_;

}; /* neuron iaf_wang_2002 */

inline size_t
iaf_wang_2002::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_wang_2002::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( not( INF_SPIKE_RECEPTOR < receptor_type and receptor_type < SUP_SPIKE_RECEPTOR ) )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
    return 0;
  }
  else
  {
    return receptor_type;
  }
}

inline size_t
iaf_wang_2002::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_wang_2002::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
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
iaf_wang_2002::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_wang_2002::set_status( const DictionaryDatum& d )
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
#endif // IAF_WANG_2002
