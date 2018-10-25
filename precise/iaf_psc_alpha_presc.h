/*
 *  iaf_psc_alpha_presc.h
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

#ifndef IAF_PSC_ALPHA_PRESC_H
#define IAF_PSC_ALPHA_PRESC_H

// C++ includes:
#include <vector>

// Generated includes:
#include "config.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/** @BeginDocumentation
Name: iaf_psc_alpha_presc - Leaky integrate-and-fire neuron
with alpha-shape postsynaptic currents; prescient implementation.

Description:

iaf_psc_alpha_presc is the "prescient" implementation of the leaky
integrate-and-fire model neuron with alpha-shaped postsynaptic
currents in the sense of [1].

PSCs are normalized to an amplitude of 1pA.

The prescient implementation predicts the effect of spikes arriving
during a time step by exactly integrating their effect from the
precise time of spike arrival to the end of the time step.  This is
exact if the neuron was not refractory at the beginning of the
interval and remains subthreshold throughout the
interval. Subthreshold dynamics are integrated using exact integration
between events [2].

Parameters:

The following parameters can be set in the status dictionary.


V_m          double - Membrane potential in mV
E_L          double - Resting membrane potential in mV.
V_min        double - Absolute lower value for the membrane potential.
C_m          double - Capacity of the membrane in pF
tau_m        double - Membrane time constant in ms.
t_ref        double - Duration of refractory period in ms.
V_th         double - Spike threshold in mV.
V_reset      double - Reset potential of the membrane in mV.
tau_syn      double - Rise time of the synaptic alpha function in ms.
I_e          double - Constant external input current in pA.
Interpol_Order  int - Interpolation order for spike time:
                      0-none, 1-linear, 2-quadratic, 3-cubic

Remarks:

Please note that this node is capable of sending precise spike times
to target nodes (on-grid spike time plus offset). If this node is
connected to a spike_detector, the property "precise_times" of the
spike_detector has to be set to true in order to record the offsets
in addition to the on-grid spike times.

If tau_m is very close to tau_syn, the model will numerically behave
as if tau_m is equal to tau_syn, to avoid numerical instabilities.
For details, please see IAF_Neruons_Singularity.ipynb in
the NEST source code (docs/model_details).

References:

[1] Morrison A, Straube S, Plesser H E, & Diesmann M (2006) Exact Subthreshold
Integration with Continuous Spike Times in Discrete Time Neural Network
Simulations. To appear in Neural Computation.
[2] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
systems with applications to neuronal modeling. Biologial Cybernetics
81:381-402.

Author: Diesmann, Eppler, Morrison, Plesser, Straube

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

SeeAlso: iaf_psc_alpha, iaf_psc_alpha_canon, iaf_psc_delta_canon
*/
class iaf_psc_alpha_presc : public Archiving_Node
{
public:
  /** Basic constructor.
      This constructor should only be used by GenericModel to create
      model prototype instances.
  */
  iaf_psc_alpha_presc();

  /** Copy constructor.
      GenericModel::allocate_() uses the copy constructor to clone
      actual model instances from the prototype instance.

      @note The copy constructor MUST NOT be used to create nodes based
      on nodes that have been placed in the network.
  */
  iaf_psc_alpha_presc( const iaf_psc_alpha_presc& );

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
   * only through a Node*. Node::init() etc are public virtual
   * functions in class Node, so they are publicly accessible
   * through Node pointers.
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
   * queue, which is marked by a weight that is GSL_NAN.  This greatly
   * simplifies the code.
   *
   * For steps, during which no events occur, the precomputed propagator matrix
   * is used.  For other steps, the propagator matrix is computed as needed.
   *
   * While the neuron is refractory, membrane potential (y3_) is
   * clamped to U_reset_.
   */
  void update( Time const& origin, const long from, const long to );

  //@}

  /**
   * Compute membrane potential after return from refractoriness.
   */
  double update_y3_delta_() const;

  /** @name Threshold-crossing interpolation
   * These functions determine the time of threshold crossing using
   * interpolation, one function per interpolation
   * order. thresh_find() is the driver function and the only one to
   * be called directly.
   */
  //@{

  /** Interpolation orders. */
  enum interpOrder
  {
    NO_INTERPOL,
    LINEAR,
    QUADRATIC,
    CUBIC,
    END_INTERP_ORDER
  };

  /**
   * Localize threshold crossing.
   * Driver function to invoke the correct interpolation function
   * for the chosen interpolation order.
   * @param   Time const &
   * @returns time from previous event to threshold crossing
   */
  double thresh_find_( const double ) const;
  double thresh_find1_( const double ) const;
  double thresh_find2_( const double ) const;
  double thresh_find3_( const double ) const;
  //@}

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_alpha_presc >;
  friend class UniversalDataLogger< iaf_psc_alpha_presc >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {

    /** Membrane time constant in ms. */
    double tau_m_;

    /** Time constant of synaptic current in ms. */
    double tau_syn_;

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

    /** Interpolation order */
    interpOrder Interpol_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double y0_; //!< external input current
    double y1_; //!< alpha current, first component
    double y2_; //!< alpha current, second component
    double y3_; //!< Membrane pot. rel. to resting pot. E_L_.

    long r_; //!< refractory steps remaining

    long last_spike_step_;     //!< time stamp of most recent spike
    double last_spike_offset_; //!< offset of most recent spike

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
    Buffers_( iaf_psc_alpha_presc& );
    Buffers_( const Buffers_&, iaf_psc_alpha_presc& );

    RingBuffer spike_y1_; //!< first alpha component
    RingBuffer spike_y2_; //!< second alpha component
    RingBuffer spike_y3_; //!< membrane potential
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_alpha_presc > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    double y0_before_; //!< y0_ at beginning of mini-step, for interpolation
    double y1_before_; //!< y1_ at beginning of mini-step, for interpolation
    double y2_before_; //!< y2_ at beginning of mini-step, for interpolation
    double y3_before_; //!< y3_ at beginning of mini-step, for interpolation

    double h_ms_;            //!< time resolution in ms
    double PSCInitialValue_; //!< e / tau_syn
    double gamma_;           //!< 1/c_m * 1/(1/tau_syn - 1/tau_m)
    double gamma_sq_;        //!< 1/c_m * 1/(1/tau_syn - 1/tau_m)^2
    double expm1_tau_m_;     //!< exp(-h/tau_m) - 1
    double expm1_tau_syn_;   //!< exp(-h/tau_syn) - 1
    double P30_;             //!< progagator matrix elem, 3rd row
    double P31_;             //!< progagator matrix elem, 3rd row
    double P32_;             //!< progagator matrix elem, 3rd row

    long refractory_steps_; //!< refractory time in steps remaining
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.y3_ + P_.E_L_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_alpha_data
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
  static RecordablesMap< iaf_psc_alpha_presc > recordablesMap_;
};


inline port
nest::iaf_psc_alpha_presc::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_alpha_presc::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_alpha_presc::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_alpha_presc::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_alpha_presc::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_alpha_presc::set_status( const DictionaryDatum& d )
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

#endif // IAF_PSC_ALPHA_PRESC_H
