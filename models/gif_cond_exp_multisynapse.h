/*
 *  gif_cond_exp_multisynapse.h
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

#ifndef GIF_COND_EXP_MULTISYNAPSE_H
#define GIF_COND_EXP_MULTISYNAPSE_H

#include "config.h"

#ifdef HAVE_GSL

// Includes from gnu gsl:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

// Includes from nestkernel:
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"

#include "nest.h"

namespace nest
{

extern "C" int
gif_cond_exp_multisynapse_dynamics( double, const double*, double*, void* );

/** @BeginDocumentation
Name: gif_cond_exp_multisynapse - Conductance-based generalized
integrate-and-fire neuron model with multiple synaptic time constants
according to Mensi et al. (2012) and Pozzorini et al. (2015).

Description:

gif_cond_exp_multisynapse is the generalized integrate-and-fire neuron
according to Mensi et al. (2012) and Pozzorini et al. (2015), with
post-synaptic conductances in the form of truncated exponentials.

This model features both an adaptation current and a dynamic threshold for
spike-frequency adaptation. The membrane potential (V) is described by the
differential equation:

C*dV(t)/dt = -g_L*(V(t)-E_L) - eta_1(t) - eta_2(t) - ... - eta_n(t) + I(t)

where each eta_i is a spike-triggered current (stc), and the neuron model can
have arbitrary number of them.
Dynamic of each eta_i is described by:

tau_eta_i*d{eta_i}/dt = -eta_i

and in case of spike emission, its value increased by a constant (which can be
positive or negative):

eta_i = eta_i + q_eta_i  (in case of spike emission).

Neuron produces spikes STOCHASTICALLY according to a point process with the
firing intensity:

lambda(t) = lambda_0 * exp[ (V(t)-V_T(t)) / Delta_V ]

where V_T(t) is a time-dependent firing threshold:

V_T(t) = V_T_star + gamma_1(t) + gamma_2(t) + ... + gamma_m(t)

where gamma_i is a kernel of spike-frequency adaptation (sfa), and the neuron
model can have arbitrary number of them.
Dynamic of each gamma_i is described by:

tau_gamma_i*d{gamma_i}/dt = -gamma_i

and in case of spike emission, its value increased by a constant (which can be
positive or negative):

gamma_i = gamma_i + q_gamma_i  (in case of spike emission).

Note that in the current implementation of the model (as described in [1] and
[2]) the values of eta_i and gamma_i are affected immediately after spike
emission. However, GIF toolbox (http://wiki.epfl.ch/giftoolbox) which fits
the model using experimental data, requires a different set of eta_i and
gamma_i. It applies the jump of eta_i and gamma_i after the refractory period.
One can easily convert between q_eta/gamma of these two approaches:
q_eta_giftoolbox = q_eta_NEST * (1 - exp( -tau_ref / tau_eta ))
The same formula applies for q_gamma.

On the postsynapic side, there can be arbitrarily many synaptic time constants
(gif_psc_exp has exactly two: tau_syn_ex and tau_syn_in). This can be reached
by specifying separate receptor ports, each for a different time constant. The
port number has to match the respective "receptor_type" in the connectors.

The shape of synaptic conductance is exponential.

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
  q_stc      vector of double - Values added to spike-triggered currents (stc)
                                after each spike emission in pA.
  tau_stc    vector of double - Time constants of stc variables in ms.
  q_sfa      vector of double - Values added to spike-frequency adaptation
                                (sfa) after each spike emission in mV.
  tau_sfa    vector of double - Time constants of sfa variables in ms.
  Delta_V    double - Stochasticity level in mV.
  lambda_0   double - Stochastic intensity at firing threshold V_T in 1/s.
  V_T_star   double - Base threshold in mV.

Synaptic parameters
  tau_syn    double vector - Time constants of the synaptic conductance
                               in ms (same size as E_rev).
  E_rev      double vector - Reversal potentials in mV (same size as tau_syn).

Integration parameters
  gsl_error_tol  double - This parameter controls the admissible error of the
                          GSL integrator. Reduce it if NEST complains about
                          numerical instabilities.

Example:

neuron = nest.Create('gif_cond_exp_multisynapse',
                     params={'E_rev': [0.0, -85.0],
                             'tau_syn': [4.0, 8.0]})

spike = nest.Create('spike_generator', params={'spike_times':
                                               np.array([10.0])})

delays = [1., 30.]
w = [1., 5.]
for syn in range(2):
    nest.Connect(spike, neuron, syn_spec={'model': 'static_synapse',
                                          'receptor_type': 1 + syn,
                                          'weight': w[syn],
                                          'delay': delays[syn]})
nest.Simulate(100.)


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

SeeAlso: pp_psc_delta, gif_cond_exp, iaf_psc_exp_multisynapse,
gif_psc_exp_multisynapse
*/
class gif_cond_exp_multisynapse : public Archiving_Node
{

public:
  gif_cond_exp_multisynapse();
  gif_cond_exp_multisynapse( const gif_cond_exp_multisynapse& );
  ~gif_cond_exp_multisynapse();

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

  // make dynamics function quasi-member
  friend int
  gif_cond_exp_multisynapse_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< gif_cond_exp_multisynapse >;
  friend class UniversalDataLogger< gif_cond_exp_multisynapse >;

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

    /** List of spike-triggered current time constant in ms. */
    std::vector< double > tau_stc_;

    /** List of spike-triggered current jumps in nA. */
    std::vector< double > q_stc_;

    /** List of adaptive threshold time constant in ms. */
    std::vector< double > tau_sfa_;

    /** List of adaptive threshold jumps in mV. */
    std::vector< double > q_sfa_;

    /** Time constants of synaptic currents in ms */
    std::vector< double > tau_syn_;

    std::vector< double > E_rev_; //!< reversal potentials in mV

    /** External DC current. */
    double I_e_;

    /** boolean flag which indicates whether the neuron has connections */
    bool has_connections_;

    double gsl_error_tol; //!< error bound for GSL integrator

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dictionary

    //! Return the number of receptor ports
    inline size_t
    n_receptors() const
    {
      return E_rev_.size();
    }
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {

    //! Symbolic indices to the elements of the state vector y
    enum StateVecElems
    {
      V_M = 0,
      G,
      STATE_VEC_SIZE
    };

    static const size_t NUMBER_OF_FIXED_STATES_ELEMENTS = 1; //!< V_M
    static const size_t NUM_STATE_ELEMENTS_PER_RECEPTOR = 1; //!< G

    std::vector< double > y_; //!< neuron state

    double I_stim_; //!< This is piecewise constant external current
    double sfa_; //!< This is the change of the 'threshold' due to adaptation.
    double stc_; //!< Spike-triggered current.

    std::vector< double > sfa_elems_; //!< Vector of adaptation parameters.
    std::vector< double > stc_elems_; //!< Vector of spike-triggered parameters.

    //!< absolute refractory counter (no membrane potential propagation)
    unsigned int r_ref_;

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_& );

  }; // State_

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( gif_cond_exp_multisynapse& );
    Buffers_( const Buffers_&, gif_cond_exp_multisynapse& );

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< gif_cond_exp_multisynapse > logger_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
    double step_;            //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    std::vector< double >
      P_sfa_; // decay terms of spike-triggered current elements
    std::vector< double > P_stc_; // decay terms of adaptive threshold elements

    librandom::RngPtr rng_; // random number generator of my own thread

    unsigned int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  //! Read out the adaptive threshold potential
  double
  get_E_sfa_() const
  {
    return S_.sfa_;
  }

  //! Read out the spike-triggered current
  double
  get_I_stc_() const
  {
    return S_.stc_;
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
  static RecordablesMap< gif_cond_exp_multisynapse > recordablesMap_;
};

inline port
gif_cond_exp_multisynapse::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
gif_cond_exp_multisynapse::handles_test_event( SpikeEvent&,
  rport receptor_type )
{
  if ( receptor_type <= 0
    || receptor_type > static_cast< port >( P_.n_receptors() ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }

  P_.has_connections_ = true;
  return receptor_type;
}

inline port
gif_cond_exp_multisynapse::handles_test_event( CurrentEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
gif_cond_exp_multisynapse::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
gif_cond_exp_multisynapse::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
gif_cond_exp_multisynapse::set_status( const DictionaryDatum& d )
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

#endif // HAVE_GSL
#endif /* #ifndef GIF_COND_EXP_MULTISYNAPSE_H */
