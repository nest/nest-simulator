/*
 *  gif_psc_exp.h
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

#ifndef GIF_PSC_EXP_H
#define GIF_PSC_EXP_H

// Includes from nestkernel:
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"

#include "nest.h"

namespace nest
{

/* BeginUserDocs: neuron, integrate-and-fire, current-based

Short description
+++++++++++++++++

Current-based generalized integrate-and-fire neuron model

Description
+++++++++++

gif_psc_exp is the generalized integrate-and-fire neuron according to
Mensi et al. (2012) [1]_ and Pozzorini et al. (2015) [2]_, with exponential
shaped postsynaptic currents.

This model features both an adaptation current and a dynamic threshold for
spike-frequency adaptation. The membrane potential (V) is described by the
differential equation:

.. math::

 C*dV(t)/dt = -g_L*(V(t)-E_L) - \eta_1(t) - \eta_2(t) - \ldots
    - \eta_n(t) + I(t)

where each :math:`\eta_i` is a spike-triggered current (stc), and the neuron
model can have arbitrary number of them.
Dynamic of each :math:`\eta_i` is described by:

.. math::

 \tau_\eta{_i}*d{\eta_i}/dt = -\eta_i

and in case of spike emission, its value increased by a constant (which can be
positive or negative):

.. math::

 \eta_i = \eta_i + q_{\eta_i} \text{ (in case of spike emission).}

Neuron produces spikes stochastically according to a point process with the
firing intensity:

.. math::

 \lambda(t) = \lambda_0 * \exp (V(t)-V_T(t)) / \Delta_V

where :math:`V_T(t)` is a time-dependent firing threshold:

.. math::

 V_T(t) = V_{T_{star}} + \gamma_1(t) + \gamma_2(t) + \ldots + \gamma_m(t)

where :math:`\gamma_i` is a kernel of spike-frequency adaptation (sfa), and the
neuron model can have arbitrary number of them.
Dynamic of each :math:`\gamma_i` is described by:

.. math::

   \tau_{\gamma_i}*d\gamma_i/dt = -\gamma_i

and in case of spike emission, its value increased by a constant (which can be
positive or negative):

.. math::

   \gamma_i = \gamma_i + q_{\gamma_i}  \text{ (in case of spike emission).}


Note:

In the current implementation of the model, the values of
:math:`\eta_i` and :math:`\gamma_i` are affected immediately after spike
emission. However, `GIF toolbox <http://wiki.epfl.ch/giftoolbox>`_, which
fits the model using experimental data, requires a different set of
:math:`\eta_i` and :math:`\gamma_i`. It applies the jump of
:math:`\eta_i` and :math:`\gamma_i` after the refractory period. One can
easily convert between :math:`q_\eta/\gamma` of these two approaches:

.. math::

  q{_\eta}_{giftoolbox} = q_{\eta_{NEST}} * (1 - \exp( -\tau_{ref} /
   \tau_\eta ))

The same formula applies for :math:`q_{\gamma}`.


The shape of postsynaptic current is exponential.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======== ======= ================================================
**Membrane Parameters**
-----------------------------------------------------------------
 C_m        pF    Capacitance of the membrane
 t_ref      ms    Duration of refractory period
 V_reset    mV    Membrane potential is reset to this value after
                  a spike
 E_L        mV    Resting potential
 g_L        nS    Leak conductance
 I_e        pA    Constant input current
======== ======= ================================================

=========  ========== ====================================================
**Spike adaptation and firing intensity parameters**
--------------------------------------------------------------------------
q_stc      list of nA   Values added to spike-triggered currents (stc)
                        after each spike emission
tau_stc    list of ms   Time constants of stc variables
q_sfa      list of mV   Values added to spike-frequency adaptation
                        (sfa) after each spike emission
tau_sfa    list of ms   Time constants of sfa variables
Delta_V    mV           Stochasticity level
lambda_0   1/s          Stochastic intensity at firing threshold V_T
V_T_star   mV           Base threshold
=========  ========== ====================================================

=========== ======= ===========================================================
**Synaptic parameters**
-------------------------------------------------------------------------------
 tau_syn_ex ms      Time constant of excitatory synaptic conductance
 tau_syn_in ms      Time constant of the inhibitory synaptic conductance
=========== ======= ===========================================================

References
++++++++++

.. [1] Mensi S, Naud R, Pozzorini C, Avermann M, Petersen CC, Gerstner W (2012)
       Parameter extraction and classification of three cortical neuron types
       reveals two distinct adaptation mechanisms. Journal of
       Neurophysiology, 107(6):1756-1775.
       DOI: https://doi.org/10.1152/jn.00408.2011
.. [2] Pozzorini C, Mensi S, Hagens O, Naud R, Koch C, Gerstner W (2015).
       Automated high-throughput characterization of single neurons by means of
       simplified spiking models. PLoS Computational Biology, 11(6), e1004275.
       DOI: https://doi.org/10.1371/journal.pcbi.1004275

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

pp_psc_delta, gif_psc_exp_multisynapse, gif_cond_exp, gif_cond_exp_multisynapse, gif_pop_psc_exp

EndUserDocs */

class gif_psc_exp : public ArchivingNode
{

public:
  gif_psc_exp();
  gif_psc_exp( const gif_psc_exp& );

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
  friend class RecordablesMap< gif_psc_exp >;
  friend class UniversalDataLogger< gif_psc_exp >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    double g_L_;
    double E_L_;
    double V_reset_;
    double Delta_V_;
    double V_T_star_;
    double lambda_0_; /** 1/ms */

    /** Refractory period in ms. */
    double t_ref_;

    /** Membrane capacitance in pF. */
    double c_m_;

    /** We use stc and sfa, respectively instead of eta and gamma
    (mentioned in the references). */

    /** List of spike triggered current time constant in ms. */
    std::vector< double > tau_stc_;

    /** List of spike triggered current jumps in nA. */
    std::vector< double > q_stc_;

    /** List of adaptive threshold time constant in ms. */
    std::vector< double > tau_sfa_;

    /** List of adaptive threshold jumps in mV. */
    std::vector< double > q_sfa_;

    /** Time constant of excitatory synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inhibitory synaptic current in ms. */
    double tau_in_;

    /** External DC current. */
    double I_e_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double I_stim_; //!< Piecewise constant external current
    double V_;      //!< Membrane potential
    double sfa_;    //!< Change of the 'threshold' due to adaptation.
    double stc_;    //!< Spike triggered current

    std::vector< double > sfa_elems_; //!< Vector of adaptation parameters
    std::vector< double > stc_elems_; //!< Vector of spike triggered parameters

    double I_syn_ex_; //!< Postsynaptic current for exc
    double I_syn_in_; //!< Postsynaptic current for inh

    unsigned int r_ref_; //!< Absolute refractory counter (no membrane potential propagation)

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( gif_psc_exp& );
    Buffers_( const Buffers_&, gif_psc_exp& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_ex_;
    RingBuffer spikes_in_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< gif_psc_exp > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    double P30_;   // coefficient for solving membrane potential equation
    double P33_;   // decay term of membrane potential
    double P31_;   // coefficient for solving membrane potential equation
    double P11ex_; // decay terms of excitatory synaptic currents
    double P11in_; // decay terms of inhibitory synaptic currents
    double P21ex_; // coefficient for solving membrane potential equation
    double P21in_; // coefficient for solving membrane potential equation

    std::vector< double > P_sfa_; // decay terms of spike-triggered current elements
    std::vector< double > P_stc_; // decay terms of adaptive threshold elements

    librandom::RngPtr rng_; // random number generator of my own thread

    unsigned int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.V_;
  }

  //! Read out the adaptive threshold potential
  double
  get_E_sfa_() const
  {
    return S_.sfa_;
  }

  //! Read out the spike triggered current
  double
  get_I_stc_() const
  {
    return S_.stc_;
  }

  double
  get_I_syn_ex_() const
  {
    return S_.I_syn_ex_;
  }

  double
  get_I_syn_in_() const
  {
    return S_.I_syn_in_;
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
  static RecordablesMap< gif_psc_exp > recordablesMap_;
};

inline port
gif_psc_exp::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
gif_psc_exp::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
gif_psc_exp::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
gif_psc_exp::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
gif_psc_exp::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
gif_psc_exp::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

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

#endif /* #ifndef GIF_PSC_EXP_H */
