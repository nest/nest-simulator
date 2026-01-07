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
#include "universal_data_logger_impl.h"

namespace nest
{
// Disable clang-formatting for documentation due to over-wide table.
// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, current-based, hard threshold

Short description
+++++++++++++++++

Leaky integrate-and-fire model with alpha-shaped input currents

Description
+++++++++++

``iaf_psc_alpha`` is a leaky integrate-and-fire neuron model with

* a hard threshold,
* a fixed refractory period,
* no adaptation mechanisms,
* :math:`\alpha`-shaped synaptic input currents.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The membrane potential evolves according to

.. math::

   \frac{dV_\text{m}}{dt} = -\frac{V_{\text{m}} - E_\text{L}}{\tau_{\text{m}}} + \frac{I_{\text{syn}} + I_\text{e}}{C_{\text{m}}}

where the synaptic input current :math:`I_{\text{syn}}(t)` is discussed below and :math:`I_\text{e}` is
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

The synaptic input current has an excitatory and an inhibitory component

.. math::

   I_{\text{syn}}(t) = I_{\text{syn, ex}}(t) + I_{\text{syn, in}}(t)

where

.. math::

   I_{\text{syn, X}}(t) = \sum_{j} w_j \sum_k i_{\text{syn, X}}(t-t_j^k-d_j) \;,

where :math:`j` indexes either excitatory (:math:`\text{X} = \text{ex}`)
or inhibitory (:math:`\text{X} = \text{in}`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, and :math:`d_j`
is the delay from neuron :math:`j`.

The individual post-synaptic currents (PSCs) are given by

.. math::

   i_{\text{syn, X}}(t) = \frac{e}{\tau_{\text{syn, X}}} t e^{-\frac{t}{\tau_{\text{syn, X}}}} \Theta(t)

where :math:`\Theta(x)` is the Heaviside step function. The PSCs are normalized to unit maximum, that is,

.. math::

   i_{\text{syn, X}}(t= \tau_{\text{syn, X}}) = 1 \;.

As a consequence, the total charge :math:`q` transferred by a single PSC depends
on the synaptic time constant according to

.. math::

   q = \int_0^{\infty}  i_{\text{syn, X}}(t) dt = e \tau_{\text{syn, X}} \;.

By default, :math:`V_\text{m}` is not bounded from below. To limit
hyperpolarization to biophysically plausible values, set parameter
:math:`V_{\text{min}}` as lower bound of :math:`V_\text{m}`.

.. note::

   NEST uses exact integration [1]_, [2]_ to integrate subthreshold membrane
   dynamics with maximum precision; see also [3]_.

   If :math:`\tau_\text{m}\approx \tau_{\text{syn, ex}}` or
   :math:`\tau_\text{m}\approx \tau_{\text{syn, in}}`, the model will
   numerically behave as if :math:`\tau_\text{m} = \tau_{\text{syn, ex}}` or
   :math:`\tau_\text{m} = \tau_{\text{syn, in}}`, respectively, to avoid
   numerical instabilities.

   For implementation details see the
   `IAF Integration Singularity notebook <../model_details/IAF_Integration_Singularity.ipynb>`_.


Parameters
++++++++++

The following parameters can be set in the status dictionary.

=============== ================== =============================== ========================================================================
**Parameter**   **Default**        **Math equivalent**             **Description**
=============== ================== =============================== ========================================================================
``E_L``         -70 mV             :math:`E_\text{L}`              Resting membrane potential
``C_m``         250 pF             :math:`C_{\text{m}}`            Capacity of the membrane
``tau_m``       10 ms              :math:`\tau_{\text{m}}`         Membrane time constant
``t_ref``       2 ms               :math:`t_{\text{ref}}`          Duration of refractory period
``V_th``        -55 mV             :math:`V_{\text{th}}`           Spike threshold
``V_reset``     -70 mV             :math:`V_{\text{reset}}`        Reset potential of the membrane
``tau_syn_ex``  2 ms               :math:`\tau_{\text{syn, ex}}`   Rise time of the excitatory synaptic alpha function
``tau_syn_in``  2 ms               :math:`\tau_{\text{syn, in}}`   Rise time of the inhibitory synaptic alpha function
``I_e``         0 pA               :math:`I_\text{e}`              Constant input current
``V_min``       :math:`-\infty` mV :math:`V_{\text{min}}`          Absolute lower value for the membrane potential
=============== ================== =============================== ========================================================================

The following state variables evolve during simulation and are available either as neuron properties or as recordables.

================== ================= ========================== =================================
**State variable** **Initial value** **Math equivalent**        **Description**
================== ================= ========================== =================================
``V_m``            -70 mV            :math:`V_{\text{m}}`       Membrane potential
``I_syn_ex``       0 pA              :math:`I_{\text{syn, ex}}` Excitatory synaptic input current
``I_syn_in``       0 pA              :math:`I_{\text{syn, in}}` Inhibitory synaptic input current
================== ================= ========================== =================================


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


Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_psc_alpha

EndUserDocs */
// clang-format on

void register_iaf_psc_alpha( const std::string& name );

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

inline size_t
nest::iaf_psc_alpha::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_psc_alpha::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_alpha::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_alpha::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
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

template <>
void RecordablesMap< iaf_psc_alpha >::create();

} // namespace

#endif /* #ifndef IAF_PSC_ALPHA_H */
