/*
 *  iaf_psc_exp_ps.h
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

#ifndef IAF_PSC_EXP_PS_H
#define IAF_PSC_EXP_PS_H

// C++ includes:
#include <vector>

// Generated includes:
#include "config.h"

// Includes from libnestutil:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "iaf_propagator.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "slice_ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, integrate-and-fire, current-based, precise

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron with exponential-shaped
postsynaptic currents using regula falsi method for approximation of
threshold crossing

Description
+++++++++++

``iaf_psc_exp_ps`` is the "canonical" implementation of the leaky
integrate-and-fire model neuron with exponential postsynaptic currents
that uses the regula falsi method to approximate the timing of a threshold
crossing. This is the most exact implementation available.

The canonical implementation handles neuronal dynamics in a locally
event-based manner with in coarse time grid defined by the minimum
delay in the network, see [1]_ [2]_. Incoming spikes are applied at the
precise moment of their arrival, while the precise time of outgoing
spikes is determined by regula falsi once a threshold crossing has
been detected. Return from refractoriness occurs precisely at spike
time plus refractory period.

This implementation is more complex than the plain iaf_psc_exp
neuron, but achieves much higher precision. In particular, it does not
suffer any binning of spike times to grid points. Depending on your
application, the canonical application with regula falsi may provide
superior overall performance given an accuracy goal; see [1]_ [2]_ for
details. Subthreshold dynamics are integrated using exact integration
between events [3]_.

Please note that this node is capable of sending precise spike times
to target nodes (on-grid spike time and offset).

The iaf_psc_delta_ps neuron accepts connections transmitting
CurrentEvents. These events transmit stepwise-constant currents which
can only change at on-grid times.

.. note::

  If `tau_m` is very close to `tau_syn_ex` or `tau_syn_in`, the model
  will numerically behave as if `tau_m` is equal to `tau_syn_ex` or
  `tau_syn_in`, respectively, to avoid numerical instabilities.

  For implementation details see the
  `IAF_neurons_singularity <../model_details/IAF_neurons_singularity.ipynb>`_ notebook.

For details about exact subthreshold integration, please see
:doc:`../neurons/exact-integration`.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==========  =====  ==========================================================
E_L         mV     Resting membrane potential
C_m         pF     Capacitance of the membrane
tau_m       ms     Membrane time constant
tau_syn_ex  ms     Excitatory synaptic time constant
tau_syn_in  ms     Inhibitory synaptic time constant
t_ref       ms     Duration of refractory period
V_th        mV     Spike threshold
I_e         pA     Constant input current
V_min       mV     Absolute lower value for the membrane potential
V_reset     mV     Reset value for the membrane potential
==========  =====  ==========================================================

References
++++++++++

.. [1] Morrison A, Straube S, Plesser HE & Diesmann M (2007) Exact subthreshold
       integration with continuous spike times in discrete time neural network
       simulations. Neural Comput 19, 47-79
.. [2] Hanuschkin A, Kunkel S, Helias M, Morrison A and Diesmann M (2010) A
       general and efficient method for incorporating precise spike times in
       globally timedriven simulations. Front Neuroinform 4:113
.. [3] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
       systems with applications to neuronal modeling. Biol Cybern 81:381-402

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_psc_exp, iaf_psc_alpha_ps

EndUserDocs */

class iaf_psc_exp_ps : public ArchivingNode
{
public:
  /** Basic constructor.
      This constructor should only be used by GenericModel to create
      model prototype instances.
  */
  iaf_psc_exp_ps();

  /** Copy constructor.
      GenericModel::create_() uses the copy constructor to clone
      actual model instances from the prototype instance.

      @note The copy constructor MUST NOT be used to create nodes based
      on nodes that have been placed in the network.
  */
  iaf_psc_exp_ps( const iaf_psc_exp_ps& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool ) override;

  port handles_test_event( SpikeEvent&, rport ) override;
  port handles_test_event( CurrentEvent&, rport ) override;
  port handles_test_event( DataLoggingRequest&, rport ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  bool
  is_off_grid() const override
  {
    return true;
  }

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  /**
   * Based on the current state, compute the value of the membrane potential
   * after taking a timestep of length ``t_step``, and use it to compute the
   * signed distance to spike threshold at that time. The internal state is not
   * actually updated (method is defined const).
   *
   * @param   double time step
   * @returns difference between updated membrane potential and threshold
   */
  double threshold_distance( double t_step ) const;

private:
  /** @name Interface functions
   * @note These functions are private, so that they can be accessed
   * only through a Node*.
   */
  //@{
  void init_buffers_() override;
  void pre_run_hook() override;

  /**
   * Time Evolution Operator.
   *
   * update() promotes the state of the neuron from origin+from to origin+to.
   * It does so in steps of the resolution h.  Within each step, time is
   * advanced from event to event, as retrieved from the spike queue.
   *
   * Return from refractoriness is handled as a special event in the
   * queue, which is marked by a weight that is GSL_NAN.  This greatly
   * simplifies the code.
   *
   * For steps, during which no events occur, the precomputed propagator matrix
   * is used.  For other steps, the propagator matrix is computed as needed.
   *
   * While the neuron is refractory, membrane potential (y2_) is
   * clamped to U_reset_.
   */
  void update( Time const& origin, const long from, const long to ) override;
  //@}

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_exp_ps >;
  friend class UniversalDataLogger< iaf_psc_exp_ps >;

  /**
   * Propagate neuron state.
   * Propagate the neuron's state by dt.
   * @param dt Interval over which to propagate
   */
  void propagate_( const double dt );

  /**
   * Trigger iterative method to find the precise spike time within
   * the mini-timestep (t0,t0+dt] assuming that the membrane
   * potential was below threshold at t0 and above at t0+dt. Emit
   * the spike and reset the neuron.
   *
   * @param origin  Time stamp at beginning of slice
   * @param lag     Time step within slice
   * @param t0      Beginning of mini-timestep
   * @param dt      Duration of mini-timestep
   */
  void emit_spike_( const Time& origin, const long lag, const double t0, const double dt );

  /**
   * Instantaneously emit a spike at the precise time defined by
   * origin, lag and spike_offset and reset the neuron.
   *
   * @param origin        Time stamp at beginning of slice
   * @param lag           Time step within slice
   * @param spike_offset  Time offset for spike
   */
  void emit_instant_spike_( const Time& origin, const long lag, const double spike_offset );

  /** Propagator object for updating synaptic components */
  IAFPropagatorExp propagator_ex_;
  IAFPropagatorExp propagator_in_;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Membrane time constant in ms. */
    double tau_m_;

    /** Time constant of exc. synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inh. synaptic current in ms. */
    double tau_in_;

    /** Membrane capacitance in pF. */
    double c_m_;

    /** Refractory period in ms. */
    double t_ref_;

    /** Resting potential in mV. */
    double E_L_;

    /** External DC current [pA] */
    double I_e_;

    /** Threshold, RELATIVE TO RESTING POTENTAIL(!).
        I.e. the real threshold is U_th_ + E_L_. */
    double U_th_;

    /** Lower bound, RELATIVE TO RESTING POTENTAIL(!).
        I.e. the real lower bound is U_min_+E_L_. */
    double U_min_;

    /** Reset potential.
        At threshold crossing, the membrane potential is reset to this value.
        Relative to resting potential. */
    double U_reset_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum&, Node* node );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double y0_;    //!< External input current
    double y1_ex_; //!< Excitatory synaptic current
    double y1_in_; //!< Inhibitory synaptic current
    double y2_;    //!< Membrane potential (relative to resting potential)

    bool is_refractory_;       //!< True while refractory
    long last_spike_step_;     //!< Time stamp of most recent spike
    double last_spike_offset_; //!< Offset of most recent spike

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, double, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_exp_ps& );
    Buffers_( const Buffers_&, iaf_psc_exp_ps& );

    /**
     * Queue for incoming events.
     * @note Handles also pseudo-events marking return from refractoriness.
     */
    SliceRingBuffer events_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_exp_ps > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    double h_ms_;           //!< Time resolution [ms]
    long refractory_steps_; //!< Refractory time in steps
    double expm1_tau_m_;    //!< expm1(-h/tau_m)
    double exp_tau_ex_;     //!< exp(-h/tau_ex)
    double exp_tau_in_;     //!< exp(-h/tau_in)
    double P20_;            //!< Progagator matrix element, 2nd row
    double P21_in_;         //!< Progagator matrix element, 2nd row
    double P21_ex_;         //!< Progagator matrix element, 2nd row
    double y0_before_;      //!< y0_ at beginning of ministep
    double y1_ex_before_;   //!< y1_ at beginning of ministep
    double y1_in_before_;   //!< y1_ at beginning of ministep
    double y2_before_;      //!< y2_ at beginning of ministep
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.y2_ + P_.E_L_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_exp_ps_data
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

  //! Mapping of recordables names to access functions
  static RecordablesMap< iaf_psc_exp_ps > recordablesMap_;
};

inline port
nest::iaf_psc_exp_ps::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_exp_ps::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_exp_ps::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_exp_ps::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_exp_ps::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_exp_ps::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // IAF_PSC_EXP_PS_H
