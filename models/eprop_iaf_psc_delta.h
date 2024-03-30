/*
 *  eprop_iaf_psc_delta.h
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

#ifndef EPROP_IAF_PSC_DELTA_H
#define EPROP_IAF_PSC_DELTA_H

// Includes from nestkernel:
#include "connection.h"
#include "eprop_archiving_node.h"
#include "eprop_archiving_node_impl.h"
#include "eprop_synapse.h"
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

``iaf_psc_delta`` is an implementation of a leaky integrate-and-fire model
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

The iaf_psc_delta is the standard model used to check the consistency
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
 V_min            mV      Absolute lower value for the membrane potential
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

See also
++++++++

iaf_psc_alpha, iaf_psc_exp, iaf_psc_delta_ps

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_psc_delta

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
 * optimization levels. A future version of iaf_psc_delta will probably
 * address the problem of efficient usage of appropriate vector and
 * matrix objects.
 */

void register_eprop_iaf_psc_delta( const std::string& name );

class eprop_iaf_psc_delta : public EpropArchivingNodeRecurrent
{

public:
  eprop_iaf_psc_delta();
  eprop_iaf_psc_delta( const eprop_iaf_psc_delta& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( LearningSignalConnectionEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( LearningSignalConnectionEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void compute_gradient( const long t_spike,
    const long t_previous_spike,
    double& previous_z_buffer,
    double& z_bar,
    double& e_bar,
    double& epsilon,
    double& weight,
    const CommonSynapseProperties& cp,
    WeightOptimizer* optimizer ) override;

  void pre_run_hook() override;
  long get_shift() const override;
  bool is_eprop_recurrent_node() const override;
  void update( Time const&, const long, const long ) override;

  //! Get maximum number of time steps integrated between two consecutive spikes.
  long get_eprop_isi_trace_cutoff() override;

protected:
  void init_buffers_() override;

private:
  //! Compute the surrogate gradient.
  double ( eprop_iaf_psc_delta::*compute_surrogate_gradient )( double, double, double, double, double, double );

  //! Map for storing a static set of recordables.
  friend class RecordablesMap< eprop_iaf_psc_delta >;

  //! Logger for universal data supporting the data logging request / reply mechanism. Populated with a recordables map.
  friend class UniversalDataLogger< eprop_iaf_psc_delta >;

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

    /** External DC current */
    double I_e_;

    /** Threshold, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real threshold is (E_L_+V_th_). */
    double V_th_;

    /** Lower bound, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real lower bound is (V_min_+V_th_). */
    double V_min_;

    /** reset value of the membrane potential */
    double V_reset_;

    bool with_refr_input_; //!< spikes arriving during refractory period are
                           //!< counted

    //! Prefactor of firing rate regularization.
    double c_reg_;


    //! Target firing rate of rate regularization (spikes/s).
    double f_target_;

    //! Width scaling of surrogate gradient / pseudo-derivative of membrane voltage.
    double beta_;

    //! Height scaling of surrogate gradient / pseudo-derivative of membrane voltage.
    double gamma_;

    //! Surrogate gradient / pseudo-derivative function of the membrane voltage ["piecewise_linear", "exponential",
    //! "fast_sigmoid_derivative", "arctan"]
    std::string surrogate_gradient_function_;

    //! Low-pass filter of the eligibility trace.
    double kappa_;

    //!< Number of time steps integrated between two consecutive spikes is equal to the minimum between
    //!< eprop_isi_trace_cutoff_ and the inter-spike distance.
    long eprop_isi_trace_cutoff_;

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
    double y0_;
    //! This is the membrane potential RELATIVE TO RESTING POTENTIAL.
    double y3_;

    int r_; //!< Number of refractory steps remaining

    /** Accumulate spikes arriving during refractory period, discounted for
        decay until end of refractory period.
    */
    double refr_spikes_buffer_;

    //! Learning signal. Sum of weighted error signals coming from the readout neurons.
    double learning_signal_;

    //! Surrogate gradient / pseudo-derivative of the membrane voltage.
    double surrogate_gradient_;

    //! Binary spike variable - 1.0 if the neuron has spiked in the previous time step and 0.0 otherwise.
    double z_;

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
    Buffers_( eprop_iaf_psc_delta& );
    Buffers_( const Buffers_&, eprop_iaf_psc_delta& );

    /** buffers and summs up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< eprop_iaf_psc_delta > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    double P30_;
    double P33_;

    //! Propagator matrix entry for evolving the incoming spike variables.
    double P_z_in_;

    int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.y3_ + P_.E_L_;
  }

  //! Get the current value of the surrogate gradient.
  double
  get_surrogate_gradient_() const
  {
    return S_.surrogate_gradient_;
  }

  //! Get the current value of the learning signal.
  double
  get_learning_signal_() const
  {
    return S_.learning_signal_;
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
  static RecordablesMap< eprop_iaf_psc_delta > recordablesMap_;
};

inline long
eprop_iaf_psc_delta::get_eprop_isi_trace_cutoff()
{
  return P_.eprop_isi_trace_cutoff_;
}

inline size_t
nest::eprop_iaf_psc_delta::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
eprop_iaf_psc_delta::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
eprop_iaf_psc_delta::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
eprop_iaf_psc_delta::handles_test_event( LearningSignalConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return 0;
}

inline size_t
eprop_iaf_psc_delta::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
eprop_iaf_psc_delta::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  // ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
eprop_iaf_psc_delta::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  // ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif /* #ifndef EPROP_IAF_PSC_DELTA_H */
