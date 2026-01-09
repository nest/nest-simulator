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
#include "universal_data_logger_impl.h"

namespace nest
{
// Disable clang-formatting for documentation due to over-wide table.
// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, current-based, hard threshold

Short description
+++++++++++++++++

Leaky integrate-and-fire model with delta-shaped input currents

Description
+++++++++++

``iaf_psc_delta`` is a leaky integrate-and-fire neuron model with

* a hard threshold,
* a fixed refractory period,
* Dirac delta (:math:`\delta`)-shaped synaptic input currents.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The membrane potential evolves according to

.. math::

   \frac{dV_\text{m}}{dt} = -\frac{V_{\text{m}} - E_\text{L}}{\tau_{\text{m}}} + \dot{\Delta}_{\text{syn}} + \frac{I_{\text{syn}} + I_\text{e}}{C_{\text{m}}}

where the derivative of change in voltage due to synaptic input :math:`\dot{\Delta}_{\text{syn}}(t)` is discussed below and :math:`I_\text{e}` is
a constant input current set as a model parameter.

A spike is emitted at time step :math:`t^*=t_{k+1}` if

.. math::

   V_\text{m}(t_k) < V_{th} \quad\text{and}\quad V_\text{m}(t_{k+1})\geq V_\text{th} \;.

Subsequently,

.. math::

   V_\text{m}(t) = V_{\text{reset}} \quad\text{for}\quad t^* \leq t < t^* + t_{\text{ref}} \;,

that is, the membrane potential is clamped to :math:`V_{\text{reset}}` during the refractory period.

Synaptic input
..............

The change in membrane potential due to synaptic inputs can be formulated as:

.. math::

   \dot{\Delta}_{\text{syn}}(t) = \sum_{j} w_j \sum_k \delta(t-t_j^k-d_j) \;,

where :math:`j` indexes either excitatory (:math:`w_j > 0`)
or inhibitory (:math:`w_j < 0`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, :math:`d_j`
is the delay from neuron :math:`j`, and :math:`\delta` is the Dirac delta distribution.
This implies that the jump in voltage upon a single synaptic input spike is

.. math::

   \Delta_{\text{syn}} = w \;,

where :math:`w` is the corresponding synaptic weight in mV.


The change in voltage caused by the synaptic input can be interpreted as being caused
by individual post-synaptic currents (PSCs) given by

.. math::

   i_{\text{syn}}(t) = C_{\text{m}} \cdot w \cdot \delta(t).

As a consequence, the total charge :math:`q` transferred by a single PSC is

.. math::

   q = \int_0^{\infty}  i_{\text{syn, X}}(t) dt = C_{\text{m}} \cdot w \;.

By default, :math:`V_\text{m}` is not bounded from below. To limit
hyperpolarization to biophysically plausible values, set parameter
:math:`V_{\text{min}}` as lower bound of :math:`V_\text{m}`.





.. note::

   NEST uses exact integration [1]_, [2]_ to integrate subthreshold membrane
   dynamics.

   Spikes arriving while the neuron is refractory, are discarded by
   default. If the property ``refractory_input`` is set to True, such
   spikes are added to the membrane potential at the end of the
   refractory period, dampened according to the interval between
   arrival and end of refractoriness.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==================== ================== =============================== ==================================================================================
**Parameter**        **Default**        **Math equivalent**             **Description**
==================== ================== =============================== ==================================================================================
``E_L``              -70 mV             :math:`E_\text{L}`              Resting membrane potential
``C_m``              250 pF             :math:`C_{\text{m}}`            Capacitance of the membrane
``tau_m``            10 ms              :math:`\tau_{\text{m}}`         Membrane time constant
``t_ref``            2 ms               :math:`t_{\text{ref}}`          Duration of refractory period
``V_th``             -55 mV             :math:`V_{\text{th}}`           Spike threshold
``V_reset``          -70 mV             :math:`V_{\text{reset}}`        Reset potential of the membrane
``I_e``              0 pA               :math:`I_\text{e}`              Constant input current
``V_min``            :math:`-\infty` mV :math:`V_{\text{min}}`          Absolute lower value for the membrane potential
``refractory_input`` ``False``          None                            If set to True, spikes arriving during refractory period are integrated afterwards
==================== ================== =============================== ==================================================================================


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

void register_iaf_psc_delta( const std::string& name );

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

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

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


inline size_t
nest::iaf_psc_delta::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_psc_delta::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_delta::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_delta::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
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

template <>
void RecordablesMap< iaf_psc_delta >::create();

} // namespace

#endif /* #ifndef IAF_PSC_DELTA_H */
