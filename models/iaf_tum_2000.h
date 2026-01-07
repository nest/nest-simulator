/*
 *  iaf_tum_2000.h
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

#ifndef IAF_TUM_2000_H
#define IAF_TUM_2000_H

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

// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, current-based, short-term plasticity, hard threshold

Short description
+++++++++++++++++

Leaky integrate-and-fire neuron model with exponential PSCs and integrated short-term plasticity synapse

Description
+++++++++++

``iaf_tum_2000`` is a leaky integrate-and-fire neuron model with short-term synaptic
plasticity and exponential shaped postsynaptic currents (PSCs). In particular,
``iaf_tum_2000`` implements short-term depression and short-term facilitation
according to [1]_ by solving Eqs. (3) and (4) from that paper in an exact manner.

``iaf_tum_2000`` differs from :doc:`iaf_psc_exp </models/iaf_psc_exp>` by the addition
of synaptic state variables :math:`x`, :math:`z` and :math:`u`, which together
with the membrane potential :math:`V_\text{m}` and synaptic current :math:`I_\text{syn}`
obey the following dynamics:

.. math::

   \frac{dV_\text{m}}{dt} &= -\frac{V_{\text{m}} - E_\text{L}}{\tau_{\text{m}}} + \frac{I_{\text{syn}} + I_\text{e}}{C_{\text{m}}}

   I_{\text{syn}} &= I_\text{syn,ex} + I_\text{syn,in}

   I_\text{syn,X} &= \sum_{j \in \Gamma_X} w_j y_j

   \frac{dx_j}{dt} &= \frac{z_j}{\tau_{\text{rec}}} - u_j x_j \delta(t - t_j)

   \frac{dy_j}{dt} &= -\frac{y_j}{\tau_{\text{syn},X}} + u_j x_j \delta(t - t_j)

   \frac{dz_j}{dt} &= \frac{y_j}{\tau_{\text{syn},X}} - \frac{y_j}{\tau_{\text{rec}}}

   \frac{du_j}{dt} &= -\frac{u}{\tau_{\text{fac}}} + U(1 - u) \delta(t - t_j)


where :math:`\Gamma_X` is an index set over either excitatory (:math:`\text{X} = \text{ex}`) or inhibitory (:math:`\text{X} = \text{in}`) presynaptic neurons,
:math:`k` indexes the spike times of neuron :math:`j`, and :math:`d_j`
is the delay from neuron :math:`j`.

``iaf_tum_2000`` incorporates the :doc:`tsodyks_synapse </models/tsodyks_synapse>`
computations directly in the presynaptic neuron, that is, the  synaptic state
variables :math:`x,y,z,u` are integrated in the presynaptic neuron instead of
the synapse model. For a presynaptic neuron with :math:`K` outgoing connections
following the ``tsodyks_synapse`` dynamics, ``iaf_tum_2000`` saves :math:`K-1`
integrations of the synaptic ODEs. This makes ``iaf_tum_2000`` very computationally
efficient in network simulations. Since the synaptic ODEs are linear, the
postsynaptic current can be found as the sum of all presynaptic synaptic
currents computed in the presynaptic neurons.

In order for synaptic depression or facilitation to take effect, both the
presynaptic and postsynaptic neuron must be of type ``iaf_tum_2000``.

.. note::

  Connections between ``iaf_tum_2000`` neurons must be through ``receptor_type`` 1.

.. warning::

  ``iaf_tum_2000`` does not support :ref:`precise spike timing <sim_precise_spike_times>`.
  Using precise spike timing will result in incorrect dynamics and must therefore
  be avoided.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=============== ======== =============================== ========================================================================
**Parameter**   **Unit** **Math equivalent**             **Description**
=============== ======== =============================== ========================================================================
 ``V_m``         mV       :math:`V_{\text{m}}`           Membrane potential
 ``E_L``         mV       :math:`E_\text{L}`             Resting membrane potential
 ``C_m``         pF       :math:`C_{\text{m}}`           Capacity of the membrane
 ``tau_m``       ms       :math:`\tau_{\text{m}}`        Membrane time constant
 ``t_ref``       ms       :math:`t_{\text{ref}}`         Duration of refractory period
 ``V_th``        mV       :math:`V_{\text{th}}`          Spike threshold
 ``V_reset``     mV       :math:`V_{\text{reset}}`       Reset potential of the membrane
 ``tau_syn_ex``  ms       :math:`\tau_{\text{syn, ex}}`  Excitatory synaptic time constant
 ``tau_syn_in``  ms       :math:`\tau_{\text{syn, in}}`  Inhibitory synaptic time constant
 ``U``           real     :math:`U`                      Parameter determining the increase in u with each spike [0,1]
 ``tau_fac``     ms       :math:`\tau_{\text{fac}}`      Time constant for facilitation
 ``tau_rec``     ms       :math:`\tau_{\text{rec}}`      Time constant for depression
 ``x``           real     :math:`x`                      Initial fraction of synaptic vesicles in the readily releasable pool [0,1]
 ``y``           real     :math:`y`                      Initial fraction of synaptic vesicles in the synaptic cleft [0,1]
 ``u``           real     :math:`u`                      Initial release probability of synaptic vesicles [0,1]
 ``I_e``         pA       :math:`I_\text{e}`             Constant input current
 ``V_min``       mV       :math:`V_{\text{min}}`         Absolute lower value for the membrane potenial (default :math:`-\infty`)
=============== ======== =============================== ========================================================================


References
++++++++++

.. [1] Tsodyks M, Uziel A, Markram H (2000). Synchrony generation in recurrent
       networks with frequency-dependent synapses. Journal of Neuroscience,
       20 RC50. URL: http://infoscience.epfl.ch/record/183402

Transmits
+++++++++

SpikeEvent

See also
++++++++

iaf_psc_exp, tsodyks_synapse, stdp_synapse, static_synapse

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: iaf_tum_2000

EndUserDocs */
// clang-format on

void register_iaf_tum_2000( const std::string& name );

class iaf_tum_2000 : public ArchivingNode
{

public:
  iaf_tum_2000();
  iaf_tum_2000( const iaf_tum_2000& );

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

  bool
  is_off_grid() const override
  {
    return true;
  }

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( const Time&, const long, const long ) override;

  // intensity function
  double phi_() const;

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_tum_2000 >;
  friend class UniversalDataLogger< iaf_tum_2000 >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Membrane time constant in ms. */
    double Tau_;

    /** Membrane capacitance in pF. */
    double C_;

    /** Refractory period in ms. */
    double t_ref_;

    /** Resting potential in mV. */
    double E_L_;

    /** External current in pA */
    double I_e_;

    /** Threshold, RELATIVE TO RESTING POTENTAIL(!).
        I.e. the real threshold is (E_L_+Theta_). */
    double Theta_;

    /** reset value of the membrane potential */
    double V_reset_;

    /** Time constant of excitatory synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inhibitory synaptic current in ms. */
    double tau_in_;

    /** Stochastic firing intensity at threshold in 1/s. */
    double rho_;

    /** Width of threshold region in mV. */
    double delta_;

    double tau_fac_;
    double tau_psc_;
    double tau_rec_;
    double U_;

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
    // state variables
    double i_0_;      //!< Stepwise constant input current
    double i_1_;      //!< Current input that is filtered through the excitatory synapse exponential kernel
    double i_syn_ex_; //!< Postsynaptic current for excitatory inputs (includes contribution from current input on
                      //!< receptor type 1)
    double i_syn_in_; //!< Postsynaptic current for inhibitory inputs
    double V_m_;      //!< Membrane potential
    int r_ref_;       //!< Absolute refractory counter (no membrane potential propagation)

    double x_;
    double y_;
    double u_;

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, const double, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_tum_2000& );
    Buffers_( const Buffers_&, iaf_tum_2000& );

    //! Indices for access to different channels of input_buffer_
    enum
    {
      SYN_IN = 0,
      SYN_EX,
      I0,
      I1,
      NUM_INPUT_CHANNELS
    };

    /** buffers and sums up incoming spikes/currents */
    MultiChannelInputBuffer< NUM_INPUT_CHANNELS > input_buffer_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_tum_2000 > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** Amplitude of the synaptic current.
        This value is chosen such that a postsynaptic potential with
        weight one has an amplitude of 1 mV.
        @note mog - I assume this, not checked.
    */
    //    double PSCInitialValue_;

    // time evolution operator
    double P20_;
    double P11ex_;
    double P11in_;
    double P21ex_;
    double P21in_;
    double P22_;

    double weighted_spikes_ex_;
    double weighted_spikes_in_;

    int RefractoryCounts_;

    RngPtr rng_; //!< random number generator of my own thread
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
  static RecordablesMap< iaf_tum_2000 > recordablesMap_;
};


inline size_t
nest::iaf_tum_2000::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  if ( target.get_model_id() != this->get_model_id() and target.is_off_grid() )
  {
    throw IllegalConnection( "iaf_tum_2000 neurons cannot be connected to precise spiking neurons." );
  }

  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
iaf_tum_2000::handles_test_event( SpikeEvent& e, size_t receptor_type )
{
  if ( receptor_type > 1 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  else if ( receptor_type != 1 and e.get_sender().get_model_id() == this->get_model_id() )
  {
    throw IllegalConnection( "iaf_tum_2000 neurons must be connected via receptor_type 1." );
  }
  return receptor_type;
}

inline size_t
iaf_tum_2000::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type == 0 )
  {
    return 0;
  }
  else if ( receptor_type == 1 )
  {
    return 1;
  }
  else
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
}

inline size_t
iaf_tum_2000::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_tum_2000::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_tum_2000::set_status( const DictionaryDatum& d )
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
void RecordablesMap< iaf_tum_2000 >::create();

} // namespace

#endif // IAF_TUM_2000_H
