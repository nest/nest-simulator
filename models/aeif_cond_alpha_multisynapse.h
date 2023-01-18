/*
 *  aeif_cond_alpha_multisynapse.h
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

#ifndef AEIF_COND_ALPHA_MULTISYNAPSE_H
#define AEIF_COND_ALPHA_MULTISYNAPSE_H

// Generated includes:
#include "config.h"
#include <sstream>

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
#include "ring_buffer.h"
#include "universal_data_logger.h"

/* BeginUserDocs: neuron, integrate-and-fire, adaptive threshold, conductance-based

Short description
+++++++++++++++++

Conductance based adaptive exponential integrate-and-fire neuron model

Description
+++++++++++

``aeif_cond_alpha_multisynapse`` is a conductance-based adaptive
exponential integrate-and-fire neuron model according to Brette and
Gerstner (2005) with multiple synaptic rise time and decay time
constants, and synaptic conductance modeled by an alpha function.

It allows an arbitrary number of synaptic time constants. Synaptic
conductance is modeled by an alpha function, as described by A. Roth
and M. C. W. van Rossum in Computational Modeling Methods for
Neuroscientists, MIT Press 2013, Chapter 6.

The time constants are supplied by an array, ``tau_syn``, and the pertaining
synaptic reversal potentials are supplied by the array ``E_rev``. Port numbers
are automatically assigned in the range from 1 to n_receptors.
During connection, the ports are selected with the property ``receptor_type``.

When connecting to conductance-based multisynapse models, all synaptic weights
must be non-negative.

The membrane potential is given by the following differential equation:

.. math::

 C dV/dt = -g_L(V-E_L) + g_L \cdot \Delta_T \cdot \exp((V-V_T)/\Delta_T)
 + I_{syn_{tot}}(V, t)- w + I_e

where

.. math::

 I_{syn_{tot}}(V,t) = \sum_i g_i(t) (V - E_{rev,i}) ,

the synapse `i` is excitatory or inhibitory depending on the value of
:math:`E_{rev,i}` and the differential equation for the
spike-adaptation current `w` is

.. math::

 \tau_w \cdot dw/dt = a(V - E_L) - w

When the neuron fires a spike, the adaptation current :math:`w <- w + b`.

For implementation details see the
`aeif_models_implementation <../model_details/aeif_models_implementation.ipynb>`_ notebook.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======== ======= =======================================
**Dynamic state variables:**
--------------------------------------------------------
 V_m     mV      Membrane potential
 w       pA      Spike-adaptation current
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
 Delta_T mV      Slope factor
 V_th    mV      Spike initiation threshold
 V_peak  mV      Spike detection threshold
======== ======= =======================================

======== ======= ==================================
**Spike adaptation parameters**
---------------------------------------------------
 a       ns      Subthreshold adaptation
 b       pA      Spike-triggered adaptation
 tau_w   ms      Adaptation time constant
======== ======= ==================================

======== ============= ========================================================
**Synaptic parameters**
-------------------------------------------------------------------------------
E_rev    list of mV    Reversal potential
tau_syn  list of ms    Time constant of synaptic conductance
======== ============= ========================================================

============= ======= =========================================================
**Integration parameters**
-------------------------------------------------------------------------------
gsl_error_tol real    This parameter controls the admissible error of the
                      GSL integrator. Reduce it if NEST complains about
                      numerical instabilities.
============= ======= =========================================================

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

aeif_cond_alpha_multisynapse

EndUserDocs */

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
extern "C" int aeif_cond_alpha_multisynapse_dynamics( double, const double*, double*, void* );

class aeif_cond_alpha_multisynapse : public ArchivingNode
{

public:
  aeif_cond_alpha_multisynapse();
  aeif_cond_alpha_multisynapse( const aeif_cond_alpha_multisynapse& );
  ~aeif_cond_alpha_multisynapse() override;

  friend int aeif_cond_alpha_multisynapse_dynamics( double, const double*, double*, void* );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  port handles_test_event( SpikeEvent&, rport ) override;
  port handles_test_event( CurrentEvent&, rport ) override;
  port handles_test_event( DataLoggingRequest&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;
  void update( Time const&, const long, const long ) override;

  // The next three classes need to be friends to access the State_ class/member
  friend class DynamicRecordablesMap< aeif_cond_alpha_multisynapse >;
  friend class DynamicUniversalDataLogger< aeif_cond_alpha_multisynapse >;
  friend class DataAccessFunctor< aeif_cond_alpha_multisynapse >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    double V_peak_;  //!< Spike detection threshold in mV
    double V_reset_; //!< Reset Potential in mV
    double t_ref_;   //!< Refractory period in ms

    double g_L;     //!< Leak Conductance in nS
    double C_m;     //!< Membrane Capacitance in pF
    double E_L;     //!< Leak reversal Potential (aka resting potential) in mV
    double Delta_T; //!< Slope factor in mV
    double tau_w;   //!< Adaptation time-constant in ms
    double a;       //!< Subthreshold adaptation in nS
    double b;       //!< Spike-triggered adaptation in pA
    double V_th;    //!< Spike threshold in mV

    std::vector< double > tau_syn; //!< Synaptic time constants in ms
    std::vector< double > E_rev;   //!< Reversal potentials in mV

    double I_e; //!< Intrinsic current in pA

    double gsl_error_tol; //!< error bound for GSL integrator

    // boolean flag which indicates whether the neuron has connections
    bool has_connections_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary

    //! Return the number of receptor ports
    inline size_t
    n_receptors() const
    {
      return E_rev.size();
    }
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor required because of C-style arrays.
   */
  struct State_
  {
    /**
     * Enumeration identifying elements in state vector State_::y_.
     * This enum identifies the elements of the vector. It must be public to be
     * accessible from the iteration function. The last two elements of this
     * enum (DG, G) will be repeated
     * n times at the end of the state vector State_::y with n being the number
     * of synapses.
     */
    enum StateVecElems
    {
      V_M = 0,
      W,  // 1
      DG, // 2
      G,  // 3
      STATE_VECTOR_MIN_SIZE
    };

    static const size_t NUMBER_OF_FIXED_STATES_ELEMENTS = 2; // V_M, W
    static const size_t NUM_STATE_ELEMENTS_PER_RECEPTOR = 2; // DG, G

    std::vector< double > y_; //!< neuron state
    int r_;                   //!< number of refractory steps remaining

    State_( const Parameters_& ); //!< Default initialization

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, Node* node );

  }; // State_

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( aeif_cond_alpha_multisynapse& );
    Buffers_( const Buffers_&, aeif_cond_alpha_multisynapse& );

    //! Logger for all analog data
    DynamicUniversalDataLogger< aeif_cond_alpha_multisynapse > logger_;

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spikes_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // Since IntegrationStep_ is initialized with step_, and the resolution
    // cannot change after nodes have been created, it is safe to place both
    // here.
    double step_;            //!< simulation step size in ms
    double IntegrationStep_; //!< current integration time step,
                             //!< updated by solver

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
    /** initial value to normalise synaptic conductance */
    std::vector< double > g0_;

    /**
     * Threshold detection for spike events: P.V_peak if Delta_T > 0.,
     * P.V_th if Delta_T == 0.
     */
    double V_peak;

    unsigned int refractory_counts_;
  };

  // Data members -----------------------------------------------------------

  /**
   * @defgroup aeif_cond_alpha_multisynapse
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  // Access functions for UniversalDataLogger -------------------------------

  //! Mapping of recordables names to access functions
  DynamicRecordablesMap< aeif_cond_alpha_multisynapse > recordablesMap_;

  // Data Access Functor getter
  DataAccessFunctor< aeif_cond_alpha_multisynapse > get_data_access_functor( size_t elem );
  inline double
  get_state_element( size_t elem )
  {
    return S_.y_[ elem ];
  };

  // Utility function that inserts the synaptic conductances to the
  // recordables map

  Name get_g_receptor_name( size_t receptor );
  void insert_conductance_recordables( size_t first = 0 );
};

inline port
aeif_cond_alpha_multisynapse::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
aeif_cond_alpha_multisynapse::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_cond_alpha_multisynapse::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
aeif_cond_alpha_multisynapse::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

} // namespace

#endif // HAVE_GSL
#endif // AEIF_COND_ALPHA_MULTISYNAPSE_H //
