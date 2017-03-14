/*
 *  iaf_psc_delta_canon.h
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

#ifndef IAF_PSC_DELTA_CANON_H
#define IAF_PSC_DELTA_CANON_H

// Generated includes:
#include "config.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

// Includes from precise:
#include "slice_ring_buffer.h"

namespace nest
{
/* BeginDocumentation
   Name: iaf_psc_delta_canon - Leaky integrate-and-fire neuron model.

   Description:
   iaf_psc_delta_canon is an implementation of a leaky integrate-and-fire model
   where the potential jumps on each spike arrival.

   The threshold crossing is followed by an absolute refractory period
   during which the membrane potential is clamped to the resting
   potential.

   Spikes arriving while the neuron is refractory, are discarded by
   default. If the property "refractory_input" is set to true, such
   spikes are added to the membrane potential at the end of the
   refractory period, dampened according to the interval between
   arrival and end of refractoriness.

   The linear subthresold dynamics is integrated by the Exact
   Integration scheme [1]. The neuron dynamics are solved exactly in
   time. Incoming and outgoing spike times are handled precisely [3].

   An additional state variable and the corresponding differential
   equation represents a piecewise constant external current.

   Spikes can occur either on receipt of an excitatory input spike, or
   be caused by a depolarizing input current.  Spikes evoked by
   incoming spikes, will occur precisely at the time of spike arrival,
   since incoming spikes are modeled as instantaneous potential
   jumps. Times of spikes caused by current input are determined
   exactly by solving the membrane potential equation. Note that, in
   contrast to the neuron models discussed in [3,4], this model has so
   simple dynamics that no interpolation or iterative spike location
   technique is required at all.

   The general framework for the consistent formulation of systems with
   neuron like dynamics interacting by point events is described in
   [1].  A flow chart can be found in [2].

   Critical tests for the formulation of the neuron model are the
   comparisons of simulation results for different computation step
   sizes. sli/testsuite/nest contains a number of such tests.

   The iaf_psc_delta_canon is the standard model used to check the consistency
   of the nest simulation kernel because it is at the same time complex
   enough to exhibit non-trivial dynamics and simple enough compute
   relevant measures analytically.

   Remarks:

   The iaf_psc_delta_canon neuron accepts CurrentEvent connections.
   However, the present method for transmitting CurrentEvents in
   NEST (sending the current to be applied) is not compatible with off-grid
   currents, if more than one CurrentEvent-connection exists. Once CurrentEvents
   are changed to transmit change-of-current-strength, this problem will
   disappear and the canonical neuron will also be able to handle CurrentEvents.

   The present implementation uses individual variables for the
   components of the state vector and the non-zero matrix elements of
   the propagator.  Because the propagator is a lower triangular matrix
   no full matrix multiplication needs to be carried out and the
   computation can be done "in place" i.e. no temporary state vector
   object is required.

   The template support of recent C++ compilers enables a more succinct
   formulation without loss of runtime performance already at minimal
   optimization levels. A future version of iaf_psc_delta_canon will probably
   address the problem of efficient usage of appropriate vector and
   matrix objects.

   Please note that this node is capable of sending precise spike times
   to target nodes (on-grid spike time plus offset). If this node is
   connected to a spike_detector, the property "precise_times" of the
   spike_detector has to be set to true in order to record the offsets
   in addition to the on-grid spike times.

   Parameters:
   The following parameters can be set in the status dictionary.

   V_m        double - Membrane potential in mV
   E_L        double - Resting membrane potential in mV.
   C_m        double - Capacitance of the membrane in pF
   tau_m      double - Membrane time constant in ms.
   t_ref      double - Duration of refractory period in ms.
   V_th       double - Spike threshold in mV.
   V_reset    double - Reset potential of the membrane in mV.
   I_e        double - Constant input current in pA.
   V_min      double - Absolute lower value for the membrane potential in mV.

   refractory_input bool - If true, do not discard input during
   refractory period. Default: false.

   References:
   [1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
   systems with applications to neuronal modeling. Biologial Cybernetics
   81:381-402.
   [2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space
   analysis of synchronous spiking in cortical neural networks.
   Neurocomputing 38-40:565-571.
   [3] Morrison A, Straube S, Plesser H E, & Diesmann M (2006) Exact
   Subthreshold Integration with Continuous Spike Times in Discrete Time Neural
   Network Simulations. To appear in Neural Computation.
   [4] Hanuschkin A, Kunkel S, Helias M, Morrison A & Diesmann M (2010)
   A general and efficient method for incorporating exact spike times in
   globally time-driven simulations Front Neuroinformatics, 4:113

   Sends: SpikeEvent

   Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

   Author:  May 2006, Plesser; based on work by Diesmann, Gewaltig, Morrison,
   Straube, Eppler
   SeeAlso: iaf_psc_delta, iaf_psc_exp_ps
*/

class iaf_psc_delta_canon : public Archiving_Node
{

public:
  /** Basic constructor.
      This constructor should only be used by GenericModel to create
      model prototype instances.
  */
  iaf_psc_delta_canon();

  /** Copy constructor.
      GenericModel::allocate_() uses the copy constructor to clone
      actual model instances from the prototype instance.

      @note The copy constructor MUST NOT be used to create nodes based
      on nodes that have been placed in the network.
  */
  iaf_psc_delta_canon( const iaf_psc_delta_canon& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
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
  void update( Time const&, const long, const long );

  /**
   * Calculate the precise spike time, emit the spike and reset the
   * neuron.
   *
   * @param origin    Time stamp at beginning of slice
   * @param lag       Time step within slice
   * @param offset_U  Time offset for U value, i.e. for time when threshold
   *                  crossing was detected
   */
  void emit_spike_( Time const& origin, const long lag, const double offset_U );

  /**
   * Instantaneously emit a spike at the precise time defined by
   * origin, lag and spike_offset and reset the neuron.
   *
   * @param origin        Time stamp at beginning of slice
   * @param lag           Time step within slice
   * @param spike_offset  Time offset for spike
   */
  void emit_instant_spike_( Time const& origin,
    const long lag,
    const double spike_offset );

  /**
   * Propagate neuron state.
   * Propagate the neuron's state by dt.
   * @param dt Interval over which to propagate
   */
  void propagate_( const double dt );

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {

    /** Membrane time constant in ms. */
    double tau_m_;

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
        Relative to resting potential.
    */
    double U_reset_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum& );
  };


  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_delta_canon >;
  friend class UniversalDataLogger< iaf_psc_delta_canon >;

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    //! This is the membrane potential RELATIVE TO RESTING POTENTIAL.
    double U_;
    double I_; //!< This is the current to be applied during this time step

    //! step of last spike, for reporting in status dict
    long last_spike_step_;
    double last_spike_offset_; //!< offset of last spike, for reporting in
                               //!< status dict

    bool is_refractory_;   //!< flag for refractoriness
    bool with_refr_input_; //!< spikes arriving during refractory period are
                           //!< counted

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, double );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_delta_canon& );
    Buffers_( const Buffers_&, iaf_psc_delta_canon& );

    /**
     * Queue for incoming events.
     * @note Return from refractoriness is stored as events "spikes"
     *       with weight == numerics::NaN
     */
    SliceRingBuffer events_;

    /**
     * Queue for incoming current events.
     */
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_delta_canon > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    double exp_t_;     //!< @$ e^{-t/\tau_m} @$
    double expm1_t_;   //!< @$ e^{-t/\tau_m} - 1 @$
    double v_inf_;     //!< @$ \frac{I_e\tau_m}{c_m} @$
    double I_contrib_; //!< @$ \frac{I_e\tau_m}{c_m} (1-e^{-t/\tau_m})@$

    double h_ms_; //!< duration of time step [ms]

    long refractory_steps_; //!< refractory time in steps

    /** Accumulate spikes arriving during refractory period, discounted for
        decay until end of refractory period.
    */
    double refr_spikes_buffer_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.U_ + P_.E_L_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_delta_data
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
  static RecordablesMap< iaf_psc_delta_canon > recordablesMap_;
};


inline port
nest::iaf_psc_delta_canon::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_delta_canon::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_delta_canon::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_delta_canon::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_delta_canon::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_delta_canon::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                 // temporary copy in case of errors
  const double delta_EL = ptmp.set( d ); // throws if BadProperty
  State_ stmp = S_;                      // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL );         // throws if BadProperty

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

#endif // IAF_PSC_DELTA_CANON_H
