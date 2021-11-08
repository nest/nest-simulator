/*
 *  iaf_psc_delta.h
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

#ifndef IAF_PSC_DELTA_H
#define IAF_PSC_DELTA_H

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

iaf_psc_delta is an implementation of a leaky integrate-and-fire model
where the potential jumps on each spike arrival.

The threshold crossing is followed by an absolute refractory period
during which the membrane potential is clamped to the resting potential.

Spikes arriving while the neuron is refractory, are discarded by
default. If the property "refractory_input" is set to true, such
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

Remarks:

The present implementation uses individual variables for the
components of the state vector and the non-zero matrix elements of
the propagator. Because the propagator is a lower triangular matrix,
no full matrix multiplication needs to be carried out and the
computation can be done "in place", i.e. no temporary state vector
object is required.

The template support of recent C++ compilers enables a more succinct
formulation without loss of runtime performance already at minimal
optimization levels. A future version of iaf_psc_delta will probably
address the problem of efficient usage of appropriate vector and
matrix objects.

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

See also
++++++++

iaf_psc_alpha, iaf_psc_exp, iaf_psc_delta_ps

EndUserDocs */

class iaf_psc_delta : public ArchivingNode
{

public:
  iaf_psc_delta();
  iaf_psc_delta( const iaf_psc_delta& );

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
  void calibrate();

  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_delta >;
  friend class UniversalDataLogger< iaf_psc_delta >;

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

    /** Threshold, RELATIVE TO RESTING POTENTAIL(!).
        I.e. the real threshold is (E_L_+V_th_). */
    double V_th_;

    /** Lower bound, RELATIVE TO RESTING POTENTAIL(!).
        I.e. the real lower bound is (V_min_+V_th_). */
    double V_min_;

    /** reset value of the membrane potential */
    double V_reset_;

    bool with_refr_input_; //!< spikes arriving during refractory period are
                           //!< counted

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
    Buffers_( iaf_psc_delta& );
    Buffers_( const Buffers_&, iaf_psc_delta& );

    /** buffers and summs up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_delta > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    double P30_;
    double P33_;

    int RefractoryCounts_;
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
  static RecordablesMap< iaf_psc_delta > recordablesMap_;
};


inline port
nest::iaf_psc_delta::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_delta::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_delta::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_delta::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_delta::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_delta::set_status( const DictionaryDatum& d )
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

#endif /* #ifndef IAF_PSC_DELTA_H */
