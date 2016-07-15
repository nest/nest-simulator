/*
 *  gif_psc_exp_multisynapse.h
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

#ifndef GIF_PSC_EXP_MULTISYNAPSE_H
#define GIF_PSC_EXP_MULTISYNAPSE_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "event.h"
#include "universal_data_logger.h"

#include "nest.h"

/* BeginDocumentation
  Name: gif_psc_exp_multisynapse - Current based generalized integrate-and-fire
  neuron model according to Mensi et al. (2012)
  and Pozzorini et al. (2015)

  Description:

  gif_psc_exp_multisynapse is the generalized integrate-and-fire neuron
  according to Mensi et al. (2012)
  and Pozzorini et al. (2015), with exponential shaped postsynaptic currents.

  This model features both an adaptation current and a dynamic threshold for
  spike-frequency
  adaptation. The membrane potential (V) is described by the differential
  equation:

  C*dV(t)/dt = -g_L*(V(t)-E_L) - eta_1(t) - eta_2(t) - ... - eta_n(t) + I(t)

  where each eta_i is a spike triggered current (stc), and the neuron model can
  have arbitrary number of them.
  Dynamic of each eta_i is described by:

  Tau_eta_i*d{eta_i}/dt = -eta_i

  and in case of spike emission, its value increased by a constant (which can be
  positive or negative):

  eta_i = eta_i + q_eta_i  (in case of spike emission).

  Neuron produces spikes STOCHASTICALLY according to a point process with the
  firing intensity:

  lambda(t) = lambda_0 * exp[(V(t)-V_T(t)/Delta_V)]

  where V_T(t) is a time-dependent firing threshold:

  V_T(t) = V_T_star + gamma_1(t) + gamma_2(t) + ... + gamma_m(t)

  where gamma_i is a kernel of spike-frequency adaptation (sfa), and the neuron
  model can have arbitrary number of them.
  Dynamic of each gamma_i is described by:

  Tau_gamma_i*d{gamma_i}/dt = -gamma_i

  and in case of spike emission, its value increased by a constant (which can be
  positive or negative):

  gamma_i = gamma_i + q_gamma_i  (in case of spike emission).

  On the postsynapic side, there can be arbitrarily many synaptic time constants
  (gif_psc_exp has exactly
  two: tau_syn_ex and tau_syn_in). This can be reached by specifying separate
  receptor ports, each for a
  different time constant. The port number has to match the respective
  "receptor_type" in the connectors.


  Parameters:
  The following parameters can be set in the status dictionary.

  Membrane Parameters:
    C_m        double - Capacity of the membrane in pF
    t_ref      double - Duration of refractory period in ms.
    V_reset    double - Reset value after a spike in mV.
    E_L        double - Leak reversal potential in mV.
    g_L        double - Leak conductance in nS.
    I_e        double - Constant external input current in pA.

  Spike adaptation and firing intensity parameters:
    q_stc      vector of double - Values added to spike triggered currents (stc)
  after each spike emission in nA.
    tau_stc    vector of double - Time constants of stc variables in ms.
    q_sfa      vector of double - Values added to spike-frequency adaptation
  (sfa) after each spike emission in mV.
    tau_sfa    vector of double - Time constants of sfa variables in ms.
    Delta_V    double - Stochasticity level in mV.
    lambda_0   double - Stochastic intensity at firing threshold V_T in 1/s.
    V_T_star   double - Minimum threshold in mV

  Synaptic parameters
    taus_syn  vector of double - Time constants of the synaptic currents in ms
  (exp function).


  References:

  [1] Mensi S, Naud R, Pozzorini C, Avermann M, Petersen CC, Gerstner W (2012)
  Parameter extraction and classification of three cortical neuron types
  reveals two distinct adaptation mechanisms. J. Neurophysiol., 107(6),
  1756-1775.

  [2] Pozzorini C, Mensi S, Hagens O, Naud R, Koch C, Gerstner W (2015)
  Automated High-Throughput Characterization of Single Neurons by Means of
  Simplified Spiking Models. PLoS Comput. Biol., 11(6), e1004275.


  Sends: SpikeEvent

  Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

  Author: March 2016, Setareh
  SeeAlso: pp_psc_delta, gif_psc_exp, gif_cond_exp, gif_cond_exp_multisynapse */

namespace nest
{

class gif_psc_exp_multisynapse : public Archiving_Node
{

public:
  gif_psc_exp_multisynapse();
  gif_psc_exp_multisynapse( const gif_psc_exp_multisynapse& );

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

  void update( Time const&, const long_t, const long_t );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< gif_psc_exp_multisynapse >;
  friend class UniversalDataLogger< gif_psc_exp_multisynapse >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {

    double_t g_L_;
    double_t E_L_;
    double_t V_reset_;
    double_t Delta_V_;
    double_t V_T_star_;
    double_t lambda_0_; /** 1/ms */


    /** Refractory period in ms. */
    double_t t_ref_;

    /** Membrane capacitance in pF. */
    double_t c_m_;

    /** We use stc and sfa, respectively instead of eta and gamma
    (mentioned in the references). */

    /** List of spike triggered current time constant in ms. */
    std::vector< double_t > tau_stc_;

    /** List of spike triggered current jumps in nA. */
    std::vector< double_t > q_stc_;

    /** List of adaptive threshold time constant in ms. */
    std::vector< double_t > tau_sfa_;

    /** List of adaptive threshold jumps in mV. */
    std::vector< double_t > q_sfa_;

    /** Time constants of synaptic currents in ms */
    std::vector< double_t > tau_syn_;

    /** type is long because other types are not put through in GetStatus */
    std::vector< long > receptor_types_;
    size_t num_of_receptors_;

    /** boolean flag which indicates whether the neuron has connections */
    bool has_connections_;


    /** External DC current. */
    double_t I_e_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double_t y0_; //!< This is piecewise constant external current
    double_t
      y3_; //!< This is the membrane potential RELATIVE TO RESTING POTENTIAL.
    double_t sfa_; //!< This is the change of the 'threshold' due to adaptation.
    double_t stc_; //!< Spike triggered current.

    std::vector< double_t > sfa_elems_; //!< Vector of adaptation parameters.
    std::vector< double_t >
      stc_elems_; //!< Vector of spike triggered parameters.

    std::vector< double_t >
      i_syn_; //!< instantaneous currents of different synapses.


    int_t r_ref_; //!< absolute refractory counter (no membrane potential
                  //propagation)

    bool sfa_stc_initialized_; //!< it is true if the vectors are initialized
    bool add_stc_sfa_; //!< in case of true, the stc and sfa amplitudes should
                       //be added

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( gif_psc_exp_multisynapse& );
    Buffers_( const Buffers_&, gif_psc_exp_multisynapse& );

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< gif_psc_exp_multisynapse > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    double_t P30_;
    double_t P33_;
    double_t P31_;

    std::vector< double_t > P_sfa_;
    std::vector< double_t > P_stc_;

    unsigned int receptor_types_size_;

    std::vector< double_t >
      P11_syn_; // for updating instantaneous currents of different synapses
    std::vector< double_t > P21_syn_;

    librandom::RngPtr rng_; // random number generator of my own thread

    int_t RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out the real membrane potential
  double_t
  get_V_m_() const
  {
    return S_.y3_;
  }

  //! Read out the adaptive threshold potential
  double_t
  get_E_sfa_() const
  {
    return S_.sfa_;
  }

  //! Read out the spike triggered current
  double_t
  get_stc_() const
  {
    return S_.stc_;
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
  static RecordablesMap< gif_psc_exp_multisynapse > recordablesMap_;
};

inline port
gif_psc_exp_multisynapse::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
gif_psc_exp_multisynapse::handles_test_event( CurrentEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline port
gif_psc_exp_multisynapse::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
gif_psc_exp_multisynapse::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
gif_psc_exp_multisynapse::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, ptmp );   // throws if BadProperty

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

#endif /* #ifndef GIF_PSC_EXP_MULTISYNAPSE_H */
