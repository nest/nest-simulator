/*
 *  iaf_psc_alpha.h
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

#ifndef IAF_PSC_ALPHA_H
#define IAF_PSC_ALPHA_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, integrate-and-fire, current-based

Short description
+++++++++++++++++

Leaky integrate-and-fire neuron model

Description
+++++++++++

``iaf_psc_alpha`` is an implementation of a leaky integrate-and-fire model
with alpha-function shaped synaptic currents. Thus, synaptic currents
and the resulting postsynaptic potentials have a finite rise time.

The threshold crossing is followed by an absolute refractory period
during which the membrane potential is clamped to the resting potential.

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
sizes and the testsuite contains a number of such tests.

The ``iaf_psc_alpha`` is the standard model used to check the consistency
of the nest simulation kernel because it is at the same time complex
enough to exhibit non-trivial dynamics and simple enough compute
relevant measures analytically.

.. note::

   The present implementation uses individual variables for the
   components of the state vector and the non-zero matrix elements of
   the propagator. Because the propagator is a lower triangular matrix,
   no full matrix multiplication needs to be carried out and the
   computation can be done "in place", i.e. no temporary state vector
   object is required.

   The template support of recent C++ compilers enables a more succinct
   formulation without loss of runtime performance already at minimal
   optimization levels. A future version of iaf_psc_alpha will probably
   address the problem of efficient usage of appropriate vector and
   matrix objects.

.. note::

   If ``tau_m`` is very close to ``tau_syn_ex`` or ``tau_syn_in``, the model
   will numerically behave as if ``tau_m`` is equal to ``tau_syn_ex`` or
   ``tau_syn_in``, respectively, to avoid numerical instabilities.

   For implementation details see the
   `IAF_neurons_singularity <../model_details/IAF_neurons_singularity.ipynb>`_ notebook.

See also [3]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=========== ======  ==========================================================
 V_m        mV      Membrane potential
 E_L        mV      Resting membrane potential
 C_m        pF      Capacity of the membrane
 tau_m      ms      Membrane time constant
 t_ref      ms      Duration of refractory period
 V_th       mV      Spike threshold
 V_reset    mV      Reset potential of the membrane
 tau_syn_ex ms      Rise time of the excitatory synaptic alpha function
 tau_syn_in ms      Rise time of the inhibitory synaptic alpha function
 I_e        pA      Constant input current
 V_min      mV      Absolute lower value for the membrane potenial
=========== ======  ==========================================================

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
.. [3] Morrison A, Straube S, Plesser H E, Diesmann M (2006). Exact
       subthreshold integration with continuous spike times in discrete time
       neural network simulations. Neural Computation, in press
       DOI: https://doi.org/10.1162/neco.2007.19.1.47

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_psc_delta, iaf_psc_exp, iaf_cond_exp

EndUserDocs */

class iaf_psc_alpha : public ArchivingNode
{

public:
  iaf_psc_alpha();
  iaf_psc_alpha( const iaf_psc_alpha& );

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

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_alpha >;
  friend class UniversalDataLogger< iaf_psc_alpha >;

  // ----------------------------------------------------------------

  struct Parameters_
  {
    /** Membrane time constant in ms. */
    double Tau_;

    /** Membrane capacitance in pF. */
    double C_;

    /** Refractory period in ms. */
    double TauR_;

    /** Resting potential in mV. */
    double E_L_;

    /** External current in pA */
    double I_e_;

    /** Reset value of the membrane potential */
    double V_reset_;

    /** Threshold, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real threshold is (E_L_+Theta_). */
    double Theta_;

    /** Lower bound, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real lower bound is (LowerBound_+E_L_). */
    double LowerBound_;

    /** Time constant of excitatory synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inhibitory synaptic current in ms. */
    double tau_in_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum&, Node* node );
  };

  // ----------------------------------------------------------------

  struct State_
  {
    double y0_; //!< Constant current
    double dI_ex_;
    double I_ex_;
    double dI_in_;
    double I_in_;
    //! This is the membrane potential RELATIVE TO RESTING POTENTIAL.
    double y3_;

    int r_; //!< Number of refractory steps remaining

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, double, Node* node );
  };

  // ----------------------------------------------------------------

  struct Buffers_
  {

    Buffers_( iaf_psc_alpha& );
    Buffers_( const Buffers_&, iaf_psc_alpha& );

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
    UniversalDataLogger< iaf_psc_alpha > logger_;
  };

  // ----------------------------------------------------------------

  struct Variables_
  {

    /** Amplitude of the synaptic current.
        This value is chosen such that a postsynaptic potential with
        weight one has an amplitude of 1 mV.
     */
    double EPSCInitialValue_;
    double IPSCInitialValue_;
    int RefractoryCounts_;

    double P11_ex_;
    double P21_ex_;
    double P22_ex_;
    double P31_ex_;
    double P32_ex_;
    double P11_in_;
    double P21_in_;
    double P22_in_;
    double P31_in_;
    double P32_in_;
    double P30_;
    double P33_;
    double expm1_tau_m_;

    double weighted_spikes_ex_;
    double weighted_spikes_in_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  inline double
  get_V_m_() const
  {
    return S_.y3_ + P_.E_L_;
  }

  inline double
  get_I_syn_ex_() const
  {
    return S_.I_ex_;
  }

  inline double
  get_I_syn_in_() const
  {
    return S_.I_in_;
  }

  // Data members -----------------------------------------------------------

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
  static RecordablesMap< iaf_psc_alpha > recordablesMap_;
};

inline port
nest::iaf_psc_alpha::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_alpha::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_alpha::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_alpha::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_alpha::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_alpha::set_status( const DictionaryDatum& d )
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

#endif /* #ifndef IAF_PSC_ALPHA_H */
