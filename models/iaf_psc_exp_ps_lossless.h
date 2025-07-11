/*
 *  iaf_psc_exp_ps_lossless.h
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

#ifndef IAF_PSC_EXP_PS_LOSSLESS_H
#define IAF_PSC_EXP_PS_LOSSLESS_H

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

/* BeginUserDocs: neuron, integrate-and-fire, current-based, precise, hard threshold

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron with exponential-shaped
postsynaptic currents predicting the exact number of spikes using a
state space analysis

Description
+++++++++++

``iaf_psc_exp_ps_lossless`` is the precise state space implementation of the leaky
integrate-and-fire model neuron with exponential postsynaptic currents
that uses time reversal to detect spikes [1]_. This is the most exact
implementation available.

Time-reversed state space analysis provides a general method to solve the
threshold-detection problem for an integrable, affine or linear time
evolution. This method is based on the idea of propagating the threshold
backwards in time, and see whether it meets the initial state, rather
than propagating the initial state forward in time and see whether it
meets the threshold.

.. note::

   If ``tau_m`` is very close to ``tau_syn_ex`` or ``tau_syn_in``, the model
   will numerically behave as if ``tau_m`` is equal to ``tau_syn_ex`` or
   ``tau_syn_in``, respectively, to avoid numerical instabilities.

  For implementation details see the
  `IAF Integration Singularity notebook <../model_details/IAF_Integration_Singularity.ipynb>`_.

This model transmits precise spike times to target nodes (on-grid spike
time and offset). If this node is connected to a spike_recorder, the
property "precise_times" of the spike_recorder has to be set to true in
order to record the offsets in addition to the on-grid spike times.

The iaf_psc_delta_ps neuron accepts connections transmitting
CurrentEvents. These events transmit stepwise-constant currents which
can only change at on-grid times.

In the current implementation, tau_syn_ex and tau_syn_in must be equal.
This is because the state space would be 3-dimensional otherwise, which
makes the detection of threshold crossing more difficult [1]_.
Support for different time constants may be added in the future,
see issue #921.

For details about exact subthreshold integration, please see
:doc:`../neurons/exact-integration`.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

===========  ========  ==========================================================
 E_L         mV        Resting membrane potential
 C_m         pF/mum^2  Specific capacitance of the membrane
 tau_m       ms        Membrane time constant
 tau_syn_ex  ms        Excitatory synaptic time constant
 tau_syn_in  ms        Inhibitory synaptic time constant
 t_ref       ms        Duration of refractory period
 V_th        mV        Spike threshold
 I_e         pA        Constant input current
 V_min       mV        Absolute lower value for the membrane potential.
 V_reset     mV        Reset value for the membrane potential.
===========  ========  ==========================================================

References
++++++++++

.. [1] Krishnan J, Porta Mana P, Helias M, Diesmann M and Di Napoli E
       (2018) Perfect Detection of Spikes in the Linear Sub-threshold
       Dynamics of Point Neurons. Front. Neuroinform. 11:75.
       doi: 10.3389/fninf.2017.00075

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_psc_exp_ps

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_psc_exp_ps_lossless

EndUserDocs */

void register_iaf_psc_exp_ps_lossless( const std::string& name );

class iaf_psc_exp_ps_lossless : public ArchivingNode
{
public:
  /** Basic constructor.
      This constructor should only be used by GenericModel to create
      model prototype instances.
  */
  iaf_psc_exp_ps_lossless();

  /** Copy constructor.
      GenericModel::create_() uses the copy constructor to clone
      actual model instances from the prototype instance.

      @note The copy constructor MUST NOT be used to create nodes based
      on nodes that have been placed in the network.
  */
  iaf_psc_exp_ps_lossless( const iaf_psc_exp_ps_lossless& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  bool
  is_off_grid() const override // uses off_grid events
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
  friend class RecordablesMap< iaf_psc_exp_ps_lossless >;
  friend class UniversalDataLogger< iaf_psc_exp_ps_lossless >;

  /**
   * Propagate neuron state.
   * @param dt Interval over which to propagate
   */
  void propagate_( const double dt );

  /**
   * Emit a single spike caused by DC current in absence of spike input.
   * Emits a single spike and reset neuron given that the membrane
   * potential was below threshold at the beginning of a mini-timestep
   * and above afterwards.
   *
   * @param origin  Time stamp at beginning of slice
   * @param lag     Time step within slice
   * @param t0      Beginning of mini-timestep
   * @param dt      Duration of mini-timestep
   */
  void emit_spike_( const Time& origin, const long lag, const double t0, const double dt );

  /**
   * Emit a single spike at a precisely given time.
   *
   * @param origin        Time stamp at beginning of slice
   * @param lag           Time step within slice
   * @param spike_offset  Time offset for spike
   */
  void emit_instant_spike_( const Time& origin, const long lag, const double spike_offset );

  /**
   * Retrospective spike detection by state space analysis.
   * The state space spanning the non-spiking region is bound by the following
   * system of inequalities:
   * threshold line V < \theta, envelope, V < b(I_e), line corresponding to the
   * final timestep
   * V < f(h, I) (or) linear approximation of the envelope, V < g(h, I_e).
   * The state space spanning the spiking region is bound by the following
   * system of inequalities:
   * threshold line V < \theta, envelope, V > b(I_e) and line corresponding to
   * the final timestep
   * V > f(h, I) (or) linear approximation of the envelope, V < g(h, I_e).
   * Note that in Algorithm 1 and 2 of [1], a typo interchanges g and f.
   * @returns time interval in which threshold was crossed, or nan.
   */
  double is_spike_( const double );

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

    /** Threshold, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real threshold is U_th_ + E_L_. */
    double U_th_;

    /** Lower bound, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real lower bound is U_min_+E_L_. */
    double U_min_;

    /** Reset potential.
        At threshold crossing, the membrane potential is reset to this value.
        Relative to resting potential. */
    double U_reset_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;               //!< Store current values in dictionary
    double set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double y0_;       //!< External input current
    double I_syn_ex_; //!< Exc. exponential current
    double I_syn_in_; //!< Inh. exponential current
    double y2_;       //!< Membrane potential (relative to resting potential)

    bool is_refractory_;       //!< True while refractory
    long last_spike_step_;     //!< Time stamp of most recent spike
    double last_spike_offset_; //!< Offset of most recent spike

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, double delta_EL, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_exp_ps_lossless& );
    Buffers_( const Buffers_&, iaf_psc_exp_ps_lossless& );

    /**
     * Queue for incoming events.
     * @note Handles also pseudo-events marking return from refractoriness.
     */
    SliceRingBuffer events_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_exp_ps_lossless > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    double h_ms_;            //!< Time resolution [ms]
    long refractory_steps_;  //!< Refractory time in steps
    double expm1_tau_m_;     //!< expm1(-h/tau_m)
    double exp_tau_ex_;      //!< exp(-h/tau_ex)
    double exp_tau_in_;      //!< exp(-h/tau_in)
    double P20_;             //!< Progagator matrix element, 2nd row
    double P21_in_;          //!< Progagator matrix element, 2nd row
    double P21_ex_;          //!< Progagator matrix element, 2nd row
    double y0_before_;       //!< y0_ at beginning of ministep
    double I_syn_ex_before_; //!< I_syn_ex_ at beginning of ministep
    double I_syn_in_before_; //!< I_syn_in_ at beginning of ministep
    double y2_before_;       //!< y2_ at beginning of ministep

    /**
     * Pre-computed constants for inequality V < g(h, I_e)
     */
    //@{
    double a1_;
    double a2_;
    double a3_;
    double a4_;
    //@}

    /**
     * Pre-computed constants for inequality V < f(h, I)
     */
    //@{
    double b1_;
    double b2_;
    double b3_;
    double b4_;
    //@}

    /**
     * Pre-computed constants for inequality V < b(I_e)
     */
    //@{
    double c1_;
    double c2_;
    double c3_;
    double c4_;
    double c5_;
    double c6_;
    //@}
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.y2_ + P_.E_L_;
  }
  double
  get_I_syn_() const
  {
    return S_.I_syn_ex_ + S_.I_syn_in_;
  }
  double
  get_I_syn_ex_() const
  {
    return S_.I_syn_ex_;
  }
  double
  get_I_syn_in_() const
  {
    return S_.I_syn_in_;
  }
  // ----------------------------------------------------------------

  /**
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
  static RecordablesMap< iaf_psc_exp_ps_lossless > recordablesMap_;
};

inline size_t
iaf_psc_exp_ps_lossless::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_psc_exp_ps_lossless::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_exp_ps_lossless::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_exp_ps_lossless::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_exp_ps_lossless::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_exp_ps_lossless::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                 // temporary copy in case of errors
  double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                      // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );   // throws if BadProperty

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
#endif // IAF_PSC_EXP_PS_LOSSLESS_H
