/*
 *  ignore_and_fire.h
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

#ifndef IGNORE_AND_FIRE_H
#define IGNORE_AND_FIRE_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, integrate-and-fire, current-based

Short description
+++++++++++++++++

Current-based leaky integrate-and-fire neuron model with delta-shaped
postsynaptic currents

Description
+++++++++++

``ignore_and_fire`` is an implementation of a leaky integrate-and-fire model
where the potential jumps on each spike arrival.

The threshold crossing is followed by an absolute refractory period
during which the membrane potential is clamped to the resting potential.

Spikes arriving while the neuron is refractory, are discarded by
default. If the property ``refractory_input`` is set to true, such
spikes are added to the membrane potential at the end of the
refractory period, dampened according to the interval between
arrival and end of refractoriness.

The linear subthreshold dynamics is integrated by the Exact
Integration scheme [1]_. The neuron dynamics is solved on the time
grid given by the computation step size. Incoming as well as emitted
spikes are forced to that grid.

An additional state variable and the corresponding differential
equation represents a piecewise constant external current.

The general framework for the consistent formulation of systems with
neuron like dynamics interacting by point events is described in
[1]_.  A flow chart can be found in [2]_.

Critical tests for the formulation of the neuron model are the
comparisons of simulation results for different computation step
sizes. sli/testsuite/nest contains a number of such tests.

The ignore_and_fire is the standard model used to check the consistency
of the nest simulation kernel because it is at the same time complex
enough to exhibit non-trivial dynamics and simple enough compute
relevant measures analytically.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

================= ======= ======================================================
 V_m              mV      Membrane potential
 E_L              mV      Resting membrane potential
 C_m              pF      Capacity of the membrane
 tau_m            ms      Membrane time constant
 t_ref            ms      Duration of refractory period
 V_th             mV      Spike threshold
 V_reset          mV      Reset potential of the membrane
 I_e              pA      Constant input current
 V_min            mV      Absolute lower value for the membrane potenial
 refractory_input boolean If true, do not discard input during
                          refractory period. Default: false
================= ======= ======================================================

References
++++++++++

.. [1] Rotter S,  Diesmann M (1999). Exact simulation of
       time-invariant linear systems with applications to neuronal
       modeling. Biologial Cybernetics 81:381-402.
       DOI: https://doi.org/10.1007/s004220050570
.. [2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001). State
       space analysis of synchronous spiking in cortical neural
       networks. Neurocomputing 38-40:565-571.
       DOI: https://doi.org/10.1016/S0925-2312(01)00409-X

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

EndUserDocs */

/**
 * The present implementation uses individual variables for the
 * components of the state vector and the non-zero matrix elements of
 * the propagator. Because the propagator is a lower triangular matrix,
 * no full matrix multiplication needs to be carried out and the
 * computation can be done "in place", i.e. no temporary state vector
 * object is required.
 *
 * The template support of recent C++ compilers enables a more succinct
 * formulation without loss of runtime performance already at minimal
 * optimization levels. A future version of ignore_and_fire will probably
 * address the problem of efficient usage of appropriate vector and
 * matrix objects.
 */

class ignore_and_fire : public ArchivingNode
{

public:
  ignore_and_fire();
  ignore_and_fire( const ignore_and_fire& );

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
  void pre_run_hook();

  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< ignore_and_fire >;
  friend class UniversalDataLogger< ignore_and_fire >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Phase. */
    double phase_;

    /** Firing rate in spikes/s. */
    double rate_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    void set( const DictionaryDatum&, Node* node );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double refr_spikes_buffer_;

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, Node* node );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {

    Buffers_( ignore_and_fire& );
    Buffers_( const Buffers_&, ignore_and_fire& );

    //! Indices for access to different channels of input_buffer_
    enum
    {
      SYN_IN = 0,
      SYN_EX,
      I0,
      NUM_INPUT_CHANNELS
    };

    /** buffers and sums up incoming spikes/currents */
    MultiChannelInputBuffer< NUM_INPUT_CHANNELS > input_buffer_;

    //! Logger for all analog data
    UniversalDataLogger< ignore_and_fire > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    int phase_steps_;
    int firing_period_steps_;
  };

  // ----------------------------------------------------------------

  /**
   * @defgroup ignore_and_fire_data
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
  static RecordablesMap< ignore_and_fire > recordablesMap_;

  inline void
  calc_initial_variables_()
  {
    V_.firing_period_steps_ = Time( Time::ms( 1. / P_.rate_ * 1000. ) ).get_steps();
    V_.phase_steps_ = Time( Time::ms( P_.phase_ / P_.rate_ * 1000. ) ).get_steps();
  }

};

inline port
nest::ignore_and_fire::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
ignore_and_fire::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
ignore_and_fire::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
ignore_and_fire::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
ignore_and_fire::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
ignore_and_fire::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                // temporary copy in case of errors
  ptmp.set( d, this );                  // throws if BadProperty
  State_ stmp = S_;                     // temporary copy in case of errors
  stmp.set( d, ptmp, this );            // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

  ignore_and_fire::calc_initial_variables_();

}

} // namespace

#endif /* #ifndef IGNORE_AND_FIRE_H */
