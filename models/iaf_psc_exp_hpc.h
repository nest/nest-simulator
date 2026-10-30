/*
 *  iaf_psc_exp_hpc.h
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

#ifndef IAF_PSC_EXP_HPC_H
#define IAF_PSC_EXP_HPC_H

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
// Disable clang-formatting for documentation due to over-wide table.
// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, current-based, hard threshold

Short description
+++++++++++++++++

Leaky integrate-and-fire neuron model with exponential-shaped input currents and minimal functionality

Description
+++++++++++

``iaf_psc_exp_hpc`` is a variant of :doc:`iaf_psc_exp </models/iaf_psc_exp>` for large-scale simulations
in which the time spent per neuron and time step matters. It has the dynamics of ``iaf_psc_exp`` with

* a hard threshold,
* a fixed refractory period,
* no adaptation mechanisms,
* exponential-shaped synaptic input currents according to :footcite:p:`Tsodyks2000`,

but none of the additional functionality of ``iaf_psc_exp``:

* there is no stochastic spiking; the parameters ``delta`` and ``rho`` do not exist,
* there is no current input through ``receptor_type`` 1 that is filtered by the excitatory synapse.

For the same parameters and the same input, ``iaf_psc_exp_hpc`` produces exactly the same membrane potentials,
synaptic currents and spikes as ``iaf_psc_exp`` with :math:`\delta = 0`. It needs less memory per neuron and less
work per time step.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The membrane potential evolves according to

.. math::

   \frac{dV_\text{m}}{dt} = -\frac{V_{\text{m}} - E_\text{L}}{\tau_{\text{m}}} + \frac{I_{\text{syn}} + I_\text{e}}{C_{\text{m}}}

where the synaptic input current :math:`I_{\text{syn}}(t)` is discussed below and :math:`I_\text{e}` is
a constant input current set as a model parameter.

A spike is emitted at time step :math:`t^*=t_{k+1}` if

.. math::

   V_\text{m}(t_k) < V_{\text{th}} \quad\text{and}\quad V_\text{m}(t_{k+1})\geq V_\text{th} \;.

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

   I_{\text{syn, X}}(t) = \sum_{j} \sum_{k} i_{\text{syn, X}}(t-t_j^k-d_j) \;,

where :math:`j` indexes either excitatory (:math:`\text{X} = \text{ex}`)
or inhibitory (:math:`\text{X} = \text{in}`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, and :math:`d_j`
is the delay from neuron :math:`j`.

The individual post-synaptic currents (PSCs) are given by

.. math::

   i_{\text{syn, X}}(t) = w \cdot e^{-\frac{t}{\tau_{\text{syn, X}}}} \cdot \Theta(t)

where :math:`w` is a weight (excitatory if :math:`w > 0` or inhibitory if :math:`w < 0`), and :math:`\Theta(x)` is the Heaviside step function. The time dependent components of the PSCs are normalized to unit maximum, so that,

.. math::

   i_{\text{syn, X}}(t= 0) = w \;.

As a consequence, the total charge :math:`q` transferred by a single PSC depends
on the synaptic time constant according to

.. math::

   q = \int_0^{\infty}  i_{\text{syn, X}}(t) dt = w \cdot \tau_{\text{syn, X}} \;.


.. note::


  If ``tau_m`` is very close to ``tau_syn_ex`` or ``tau_syn_in``, the model
  will numerically behave as if ``tau_m`` is equal to ``tau_syn_ex`` or
  ``tau_syn_in``, respectively, to avoid numerical instabilities.

  NEST uses exact integration :footcite:p:`Rotter1999`, :footcite:p:`Diesmann2001` to integrate subthreshold membrane dynamics
  with  maximum precision.

  For implementation details see the
  `IAF Integration Singularity notebook <../model_details/IAF_Integration_Singularity.ipynb>`_.

Current input through ``receptor_type`` 0 is handled as a stepwise constant current input as in other iaf models,
that is, this current directly enters the membrane potential equation. Other receptor types are not supported.

For conversion between postsynaptic potentials (PSPs) and PSCs,
please refer to the ``postsynaptic_potential_to_current`` function in
`PyNEST Microcircuit: Helper Functions <https://github.com/INM-6/microcircuit-PD14-model/blob/main/PyNEST/src/microcircuit/helpers.py>`_.

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
``tau_syn_ex``  2 ms               :math:`\tau_{\text{syn, ex}}`   Time constant of the excitatory synaptic current
``tau_syn_in``  2 ms               :math:`\tau_{\text{syn, in}}`   Time constant of the inhibitory synaptic current
``I_e``         0 pA               :math:`I_\text{e}`              Constant input current
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

.. footbibliography::

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_psc_exp, iaf_psc_exp_ps

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_psc_exp_hpc

EndUserDocs */

void register_iaf_psc_exp_hpc( const std::string& name );

/**
 * Leaky integrate-and-fire neuron with exponential-shaped input currents and minimal functionality.
 *
 * The dynamics are those of iaf_psc_exp with a deterministic threshold. The stochastic spiking (delta, rho) and the
 * current input filtered by the excitatory synapse (receptor type 1) of iaf_psc_exp are left out, so that the update
 * of a neuron per time step contains only the propagation of the state, the threshold test and the input from one
 * buffer slot of three channels.
 */
class iaf_psc_exp_hpc : public ArchivingNode
{

public:
  iaf_psc_exp_hpc();
  iaf_psc_exp_hpc( const iaf_psc_exp_hpc& );

  /**
   * Import sets of overloaded virtual functions.
   *
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
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

  void get_status( Dictionary& ) const override;
  void set_status( const Dictionary& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( const Time&, const long, const long ) override;

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_exp_hpc >;
  friend class UniversalDataLogger< iaf_psc_exp_hpc >;

  // ----------------------------------------------------------------

  //! Independent parameters of the model.
  struct Parameters_
  {
    //! Membrane time constant in ms.
    double Tau_;

    //! Membrane capacitance in pF.
    double C_;

    //! Refractory period in ms.
    double t_ref_;

    //! Resting potential in mV.
    double E_L_;

    //! External current in pA.
    double I_e_;

    //! Threshold, relative to the resting potential: the real threshold is E_L_ + Theta_.
    double Theta_;

    //! Reset value of the membrane potential, relative to the resting potential.
    double V_reset_;

    //! Time constant of excitatory synaptic current in ms.
    double tau_ex_;

    //! Time constant of inhibitory synaptic current in ms.
    double tau_in_;

    Parameters_(); //!< Sets default parameter values

    void get( Dictionary& ) const; //!< Store current values in dictionary

    /**
     * Set values from dictionary.
     *
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const Dictionary&, Node* node );
  };

  // ----------------------------------------------------------------

  //! State variables of the model.
  struct State_
  {
    double i_0_;      //!< Stepwise constant input current
    double i_syn_ex_; //!< Postsynaptic current for excitatory inputs
    double i_syn_in_; //!< Postsynaptic current for inhibitory inputs
    double V_m_;      //!< Membrane potential, relative to the resting potential
    int r_ref_;       //!< Absolute refractory counter (no membrane potential propagation)

    State_(); //!< Default initialization

    void get( Dictionary&, const Parameters_& ) const;

    /**
     * Set values from dictionary.
     *
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const Dictionary&, const Parameters_&, const double, Node* );
  };

  // ----------------------------------------------------------------

  //! Buffers of the model.
  struct Buffers_
  {
    Buffers_( iaf_psc_exp_hpc& );
    Buffers_( const Buffers_&, iaf_psc_exp_hpc& );

    //! Indices for access to different channels of input_buffer_
    enum
    {
      SYN_IN = 0,
      SYN_EX,
      I0,
      NUM_INPUT_CHANNELS
    };

    //! Buffers and sums up incoming spikes and currents.
    MultiChannelInputBuffer< NUM_INPUT_CHANNELS > input_buffer_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_exp_hpc > logger_;
  };

  // ----------------------------------------------------------------

  //! Internal variables of the model.
  struct Variables_
  {
    // time evolution operator
    double P20_;
    double P11ex_;
    double P11in_;
    double P21ex_;
    double P21in_;
    double P22_;

    int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  inline double
  get_V_m_() const
  {
    return S_.V_m_ + P_.E_L_;
  }

  inline double
  get_I_syn_ex_() const
  {
    return S_.i_syn_ex_;
  }

  inline double
  get_I_syn_in_() const
  {
    return S_.i_syn_in_;
  }

  // ----------------------------------------------------------------

  /**
   * Instances of private data structures for the different types of data pertaining to the model.
   *
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  //! Mapping of recordables names to access functions
  static RecordablesMap< iaf_psc_exp_hpc > recordablesMap_;
};


inline size_t
iaf_psc_exp_hpc::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_psc_exp_hpc::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_exp_hpc::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
iaf_psc_exp_hpc::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_exp_hpc::get_status( Dictionary& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  d[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_exp_hpc::set_status( const Dictionary& d )
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

#endif // IAF_PSC_EXP_HPC_H
