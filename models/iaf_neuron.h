/*
 *  iaf_neuron.h
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

#ifndef IAF_NEURON_H
#define IAF_NEURON_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{
/* BeginDocumentation
Name: iaf_neuron - Leaky integrate-and-fire neuron model.

Description:

iaf_neuron is an implementation of a leaky integrate-and-fire model
with alpha-function shaped synaptic currents. Thus, synaptic currents
and the resulting post-synaptic potentials have a finite rise time.
The threshold crossing is followed by an absolute refractory period
during which the membrane potential is clamped to the resting potential.

The subthreshold membrane potential dynamics are given by [3]

dV_m/dt = - ( V_m - E_L ) / tau_m + I_syn(t) / C_m + I_e / C_m

where I_syn(t) is the sum of alpha-shaped synaptic currents

I_syn(t) = Sum[w_j alpha(t-t_j) for t_j in input spike times]

w_j is the synaptic weight of the connection through which the spike
at time t_j arrived. Each individual alpha-current is given by

alpha(t) = e * t/tau_s * e^{-t/tau_s} * Heaviside(t)

alpha(t=tau_s) == 1 is the maximum of the alpha-current.

The linear subthresold dynamics is integrated by the Exact
Integration scheme [1]. The neuron dynamics is solved on the time
grid given by the computation step size. Incoming as well as emitted
spikes are forced to that grid.

An additional state variable and the corresponding differential
equation represents a piecewise constant external current.

The general framework for the consistent formulation of systems with
neuron like dynamics interacting by point events is described in
[1].  A flow chart can be found in [2].

Critical tests for the formulation of the neuron model are the
comparisons of simulation results for different computation step
sizes. sli/testsuite/nest contains a number of such tests.

The iaf_neuron is the standard model used to check the consistency
of the nest simulation kernel because it is at the same time complex
enough to exhibit non-trivial dynamics and simple enough to compute
relevant measures analytically.


Parameters:

The following parameters can be set in the status dictionary.

V_m        double - Membrane potential in mV
E_L        double - Resting membrane potential in mV.
C_m        double - Capacity of the membrane in pF
tau_m      double - Membrane time constant in ms.
t_ref      double - Duration of refractory period in ms.
V_th       double - Spike threshold in mV.
V_reset    double - Reset potential of the membrane in mV.
tau_syn    double - Rise time of the excitatory synaptic alpha function in ms.
I_e        double - Constant external input current in pA.

Remarks:
If tau_m is very close to tau_syn, the model will numerically behave as if
tau_m is equal to tau_syn to avoid numerical instabilities.
For details, please see IAF_Neruons_Singularity.ipynb in
the NEST source code (docs/model_details).

References:
[1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
    systems with applications to neuronal modeling. Biologial Cybernetics
    81:381-402.
[2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space
    analysis of synchronous spiking in cortical neural networks.
    Neurocomputing 38-40:565-571.
[3] Morrison A, Straube S, Plesser H E, & Diesmann M (2007) Exact subthreshold
    integration with continuous spike times in discrete time neural network
    simulations. Neural Computation 19:47-79.

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

Author:  September 1999, Diesmann, Gewaltig
SeeAlso: iaf_psc_alpha, testsuite::test_iaf
*/

/**
 * Leaky integrate-and-fire neuron.
 */
class iaf_neuron : public Archiving_Node
{

public:
  iaf_neuron();
  iaf_neuron( const iaf_neuron& );

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
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_neuron >;
  friend class UniversalDataLogger< iaf_neuron >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Membrane capacitance in pF. */
    double C_;

    /** Membrane time constant in ms. */
    double Tau_;

    /** Time constant of synaptic current in ms. */
    double tau_syn_;

    /** Refractory period in ms. */
    double TauR_;

    /** Resting potential in mV. */
    double E_L_;

    /** Reset value of the membrane potential, in mV.
        @note Value is relative to resting potential E_L_. */
    double V_reset_;

    /** Threshold in mV.
        @note Value is relative to resting potential E_L_. */
    double Theta_;

    /** External current in pA */
    double I_e_;

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
    double y0_; //!< Constant current
    double y1_;
    double y2_;
    //! This is the membrane potential RELATIVE TO RESTING POTENTIAL.
    double y3_;

    int r_; //!< number of refractory steps remaining

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
    Buffers_( iaf_neuron& );
    Buffers_( const Buffers_&, iaf_neuron& );

    /** buffers and summs up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_neuron > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** Amplitude of the synaptic current.
              This value is chosen such that a post-synaptic potential with
              weight one has an amplitude of 1 mV.
     */
    double PSCInitialValue_;
    int RefractoryCounts_; //!< refractory time in steps

    double P11_;
    double P21_;
    double P22_;
    double P31_;
    double P32_;
    double P30_;
    double P33_;
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
   * @defgroup iaf_neuron_data
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is crucial: Moving Variables_
   *       to the very end increases simulation time for brunel-2.sli
   *       from 72s to 81s on a Mac, Intel Core 2 Duo 2.2GHz, g++ 4.0.1 -O3
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< iaf_neuron > recordablesMap_;

  /** @} */
};

inline port
iaf_neuron::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_neuron::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_neuron::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_neuron::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_neuron::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_neuron::set_status( const DictionaryDatum& d )
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

#endif /* #ifndef IAF_NEURON_H */
