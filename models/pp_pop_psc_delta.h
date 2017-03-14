/*
 *  pp_pop_psc_delta.h
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

#ifndef PP_POP_PSC_DELTA_H
#define PP_POP_PSC_DELTA_H

// Includes from librandom:
#include "binomial_randomdev.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"


namespace nest
{


/* BeginDocumentation
   Name: pp_pop_psc_delta - Population of point process neurons with leaky
                            integration of delta-shaped PSCs.

   Description:

   pp_pop_psc_delta is an effective model of a population of neurons. The
   N component neurons are assumed to be spike response models with escape
   noise, also known as generalized linear models. We follow closely the
   nomenclature of [1]. The component neurons are a special case of
   pp_psc_delta (with purely exponential rate function, no reset and no
   random dead_time). All neurons in the population share the inputs that it
   receives, and the output is the pooled spike train.

   The instantaneous firing rate of the N component neurons is defined as

      rate(t) = rho_0 * exp( (h(t) - eta(t))/delta_u ),

   where h(t) is the input potential (synaptic delta currents convolved with
   an exponential kernel with time constant tau_m), eta(t) models the effect
   of refractoriness and adaptation (the neuron's own spike train convolved with
   a sum of exponential kernels with time constants tau_eta), and delta_u
   sets the scale of the voltages.

   To represent a (homogeneous) population of N inhomogeneous renewal process
   neurons, we can keep track of the numbers of neurons that fired a certain
   number of time steps in the past. These neurons will have the same value of
   the hazard function (instantaneous rate), and we draw a binomial random
   number for each of these groups. This algorithm is thus very similar to
   ppd_sup_generator and gamma_sup_generator, see also [2].

   However, the adapting threshold eta(t) of the neurons generally makes the
   neurons non-renewal processes. We employ the quasi-renewal approximation
   [1], to be able to use the above algorithm. For the extension of [1] to
   coupled populations see [3].

   In effect, in each simulation time step, a binomial random number for each
   of the groups of neurons has to be drawn, independent of the number of
   represented neurons. For large N, it should be much more efficient than
   simulating N individual pp_psc_delta models.

   pp_pop_psc_delta emits spike events like other neuron models, but no more
   than one per time step. If several component neurons spike in the time step,
   the multiplicity of the spike event is set accordingly. Thus, to monitor
   its output, the multiplicity of the spike events has to be taken into
   account. Alternatively, the internal variable n_events gives the number of
   spikes emitted in a time step, and can be monitored using a multimeter.

   A journal article that describes the model and algorithm in detail is
   in preparation.


   References:

   [1] Naud R, Gerstner W (2012) Coding and decoding with adapting neurons:
   a population approach to the peri-stimulus time histogram.
   PLoS Compututational Biology 8: e1002711.

   [2] Deger M, Helias M, Boucsein C, Rotter S (2012) Statistical properties
   of superimposed stationary spike trains. Journal of Computational
   Neuroscience 32:3, 443-463.

   [3] Deger M, Schwalger T, Naud R, Gerstner W (2014) Fluctuations and
   information filtering in coupled populations of spiking neurons with
   adaptation. Physical Review E 90:6, 062704.


   Parameters:

   The following parameters can be set in the status dictionary.


   N                 int    - Number of represented neurons.
   tau_m             double - Membrane time constant in ms.
   C_m               double - Capacitance of the membrane in pF.
   rho_0             double - Base firing rate in 1/s.
   delta_u           double - Voltage scale parameter in mV.
   I_e               double - Constant input current in pA.
   tau_eta           list of doubles - time constants of post-spike kernel
                                       in ms.
   val_eta           list of doubles - amplitudes of exponentials in
                                       post-spike-kernel in mV.
   len_kernel        double - post-spike kernel eta is truncated after
                              max(tau_eta) * len_kernel.


   The parameters correspond to the ones of pp_psc_delta as follows.

      c_1              =  0.0
      c_2              =  rho_0
      c_3              =  1/delta_u
      q_sfa            =  val_eta
      tau_sfa          =  tau_eta
      I_e              =  I_e

      dead_time        =  simulation resolution
      dead_time_random =  False
      with_reset       =  False
      t_ref_remaining  =  0.0


   Sends: SpikeEvent

   Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

   Author: May 2014, Setareh, Deger
   SeeAlso: pp_psc_delta, ppd_sup_generator, gamma_sup_generator
*/

/**
 * Population of point process neurons with leaky integration of delta-shaped
 * PSCs.
 */


class pp_pop_psc_delta : public Node
{

public:
  pp_pop_psc_delta();
  pp_pop_psc_delta( const pp_pop_psc_delta& );

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
  friend class RecordablesMap< pp_pop_psc_delta >;
  friend class UniversalDataLogger< pp_pop_psc_delta >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {

    /** Number of neurons in the population. */
    int N_; // by Hesam

    /** Membrane time constant in ms. */
    double tau_m_;

    /** Membrane capacitance in pF. */
    double c_m_;

    /** ------------ */
    double rho_0_;

    /** ------------ */
    double delta_u_;

    /** Length of kernel */
    int len_kernel_;

    /** External DC current. */
    double I_e_;

    /** Array of time constants */
    std::vector< double > tau_eta_;

    /** -------------- */
    std::vector< double > val_eta_;

    Parameters_();                      //!< Sets default parameter values
    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {

    double y0_;
    double h_;

    std::vector< int > age_occupations_;
    std::vector< double > thetas_ages_;
    std::vector< int > n_spikes_past_;
    std::vector< int > n_spikes_ages_;
    std::vector< double > rhos_ages_;

    // ring array pointers
    int p_age_occupations_;
    int p_n_spikes_past_;

    bool initialized_; // it is true if the vectors are initialized

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
    Buffers_( pp_pop_psc_delta& );
    Buffers_( const Buffers_&, pp_pop_psc_delta& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< pp_pop_psc_delta > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {


    double P30_;
    double P33_;

    int len_eta_;
    std::vector< double > theta_kernel_;
    std::vector< double > eta_kernel_;

    double h_; //!< simulation time step in ms
    double min_double_;


    librandom::RngPtr rng_; // random number generator of my own thread

    librandom::BinomialRandomDev binom_dev_; // binomial random generator


    int DeadTimeCounts_;
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.h_;
  } // filtered input

  //! Read out the adaptive threshold potential
  double
  get_n_events_() const
  {
    return S_.n_spikes_past_[ S_.p_n_spikes_past_ ];
  } // number of generated spikes

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
  static RecordablesMap< pp_pop_psc_delta > recordablesMap_;
};

inline port
pp_pop_psc_delta::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
pp_pop_psc_delta::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
pp_pop_psc_delta::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
pp_pop_psc_delta::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
pp_pop_psc_delta::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
pp_pop_psc_delta::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, ptmp );   // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif /* #ifndef PP_POP_PSC_DELTA_H */
