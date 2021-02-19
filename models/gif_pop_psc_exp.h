/*
 *  gif_pop_psc_exp.h
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

#ifndef PP_POP_PSC_BETA_H
#define PP_POP_PSC_BETA_H

#include "nest.h"
#include "node.h"
#include "ring_buffer.h"
#include "poisson_randomdev.h"
#include "gsl_binomial_randomdev.h"
#include "universal_data_logger.h"

#ifdef HAVE_GSL

namespace nest
{


class Network;

/* BeginUserDocs: neuron, integrate-and-fire, current-based

Short description
+++++++++++++++++

Population of generalized integrate-and-fire neurons with exponential
postsynaptic currents and adaptation


Description
+++++++++++

This model simulates a population of spike-response model neurons with
multi-timescale adaptation and exponential postsynaptic currents, as
described by Schwalger et al. (2017) [1]_.

The single neuron model is defined by the hazard function

.. math::

 h(t) = \lambda_0  \exp\frac{V_m(t) - E_{\text{sfa}}(t)}{\Delta_V}

After each spike, the membrane potential :math:`V_m` is reset to
:math:`V_{\text{reset}}`. Spike frequency
adaptation is implemented by a set of exponentially decaying traces, the
sum of which is :math:`E_{\text{sfa}}`. Upon a spike, each of the adaptation traces is
incremented by the respective :math:`q_{\text{sfa}}` and decays with the respective time constant
:math:`\tau_{\text{sfa}}`.

The corresponding single neuron model is available in NEST as ``gif_psc_exp``.
The default parameters, although some are named slightly different, are not
matched in both models for historical reasons. See below for the parameter
translation.

Connecting two population models corresponds to full connectivity of every
neuron in each population. An approximation of random connectivity can be
implemented by connecting populations through a ``spike_dilutor``.


Parameters
++++++++++

The following parameters can be set in the status dictionary.


=========== ============= =====================================================
 V_reset    mV            Membrane potential is reset to this value after
                          a spike
 V_T_star   mV            Threshold level of the membrane potential
 E_L        mV            Resting potential
 Delta_V    mV            Noise level of escape rate
 C_m        pF            Capacitance of the membrane
 tau_m      ms            Membrane time constant
 t_ref      ms            Duration of refractory period
 I_e        pA            Constant input current
 N          integer       Number of neurons in the population
 len_kernel integer       Refractory effects are accounted for up to len_kernel
                          time steps
 lambda_0   1/s           Firing rate at threshold
 tau_syn_ex ms            Time constant for excitatory synaptic currents
 tau_syn_in ms            Time constant for inhibitory synaptic currents
 tau_sfa    list of ms    vector Adaptation time constants
 q_sfa      list of ms    Adaptation kernel amplitudes
 BinoRand   boolean       If True, binomial random numbers are used, otherwise
                          we use Poisson distributed spike counts
=========== ============= =====================================================


=============== ============  =============================
**Parameter translation to gif_psc_exp**
-----------------------------------------------------------
gif_pop_psc_exp  gif_psc_exp  relation
tau_m            g_L          tau_m = C_m / g_L
N                ---          use N gif_psc_exp neurons
=============== ============  =============================


References
++++++++++

.. [1] Schwalger T, Deger M, Gerstner W (2017). Towards a theory of cortical
       columns: From spiking neurons to interacting neural populations of
       finite size. PLoS Computational Biology.
       https://doi.org/10.1371/journal.pcbi.1005507


Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

gif_psc_exp, pp_pop_psc_delta, spike_dilutor

EndUserDocs */


/**
 * @note
 * As gif_pop_psc_exp represents many neurons in one node, it may send a lot
 * of spikes. In each time step, it sends at most one spike, the
 * multiplicity of which is set to the number of emitted spikes. Postsynaptic
 * neurons and devices in NEST understand this as several spikes, but
 * communication effort is reduced in simulations.
 *
 * This model uses a new algorithm to directly simulate the population activity
 * (sum of all spikes) of the population of neurons, without explicitly
 * representing each single neuron. The computational cost is largely
 * independent of the number N of neurons represented. The algorithm used
 * here is fundamentally different from and likely much faster than the one
 * used in the previously added population model pp_pop_psc_delta.
 */
class gif_pop_psc_exp : public Node
{

public:
  gif_pop_psc_exp();
  gif_pop_psc_exp( const gif_pop_psc_exp& );

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

  double escrate( const double );
  long draw_poisson( const double n_expect_ );
  long draw_binomial( const double n_expect_ );
  double adaptation_kernel( const int k );
  int get_history_size();

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< gif_pop_psc_exp >;
  friend class UniversalDataLogger< gif_pop_psc_exp >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Number of neurons in the population. */
    long N_;

    /** Membrane time constant in ms. */
    double tau_m_;

    /** Membrane capacitance in pF. */
    double c_m_;

    /** Absolute refractory period in ms */
    double t_ref_;

    /** ------------ */
    double lambda_0_;

    /** ------------ */
    double Delta_V_;

    /** Length of kernel */
    long len_kernel_;

    /** External DC current. */
    double I_e_;

    /** Amplitude of the reset kernel applied after a spike in mV */
    double V_reset_;

    /** Baseline level of the adapting threshold in mV */
    double V_T_star_;

    /** resting potential in mV */
    double E_L_;

    /** Synaptic time constants in ms. */
    double tau_syn_ex_;
    double tau_syn_in_;

    /** Array of time constants */
    std::vector< double > tau_sfa_;

    /** Array of adaptation amplitudes */
    std::vector< double > q_sfa_;

    /** Binomial random number switch */
    bool BinoRand_;

    Parameters_();                                  //!< Sets default parameter values
    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double y0_;        // DC input current
    double I_syn_ex_;  // synaptic current
    double I_syn_in_;  // synaptic current
    double V_m_;       // membrane potential
    double n_expect_;  // expected spike number
    double theta_hat_; // adapting threshold for non-refractory neurons
    long n_spikes_;    // number of spikes

    // internal switch signaling that state vectors are initialized
    bool initialized_;

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
    Buffers_( gif_pop_psc_exp& );
    Buffers_( const Buffers_&, gif_pop_psc_exp& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer ex_spikes_;
    RingBuffer in_spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< gif_pop_psc_exp > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    double R_;      // membrane resistance
    double P20_;    // membrane integration constant
    double P22_;    // membrane integration constant
    double P11_ex_; // synaptic integration constant
    double P11_in_; // synaptic integration constant
    int k_ref_;     // length of refractory period in time steps

    std::vector< double > Q30_;       // QR adaptation integration constant
    std::vector< double > Q30K_;      // QR adaptation integration constant
    std::vector< double > theta_;     // adaptation kernel
    std::vector< double > theta_tld_; // QR adaptation kernel

    double h_; // simulation time step in ms
    double min_double_;

    librandom::RngPtr rng_; // random number generator of own thread

    librandom::PoissonRandomDev poisson_dev_;   // Poisson random number generator
    librandom::GSL_BinomialRandomDev bino_dev_; // Binomial random number generator

    double x_;                     // internal variable of population dynamics
    double z_;                     // internal variable of population dynamics
    double lambda_free_;           // hazard rate for non-refractory neurons
    std::vector< double > m_;      // survival buffer
    std::vector< double > n_;      // population activity buffer
    std::vector< double > u_;      // mean of survivals
    std::vector< double > v_;      // variance of survivals
    std::vector< double > lambda_; // escape rates buffer
    std::vector< double > g_;      // adaptation variables

    int k0_; // rotating index of history buffers
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.V_m_;
  }

  //! Read out the number of generated spikes
  double
  get_n_events_() const
  {
    return S_.n_spikes_;
  }

  //! Read out the adaptation state
  double
  get_E_sfa_() const
  {
    return S_.theta_hat_;
  }

  //! Read out the expected number of spikes
  double
  get_mean_() const
  {
    return S_.n_expect_;
  }

  //! Read out the synaptic currents
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
  static RecordablesMap< gif_pop_psc_exp > recordablesMap_;
};

inline port
gif_pop_psc_exp::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
gif_pop_psc_exp::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
gif_pop_psc_exp::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
gif_pop_psc_exp::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
gif_pop_psc_exp::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  // MoD: In models derived from ArchivingNode, here get_status of the
  // parent class is called. Since this model derives from Node, and
  // not from ArchivingNode, this call has been disabled here
  // (Node does not have a comparable method).
  //  ArchivingNode::get_status(d);
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
gif_pop_psc_exp::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.

  // MoD: In models derived from ArchivingNode, here set_status of the
  // parent class is called. Since this model derives from Node, and
  // not from ArchivingNode, this call has been disabled here
  // (Node does not have a comparable method).
  //  ArchivingNode::set_status(d);

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace


#endif /* HAVE_GSL */
#endif /* #ifndef PP_POP_PSC_BETA_H */
