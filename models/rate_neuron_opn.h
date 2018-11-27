/*
 *  rate_neuron_opn.h
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

#ifndef RATE_NEURON_OPN_H
#define RATE_NEURON_OPN_H

// Generated includes:
#include "config.h"

// C++ includes:
#include <string>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "normal_randomdev.h"
#include "poisson_randomdev.h"
#include "ring_buffer.h"
#include "recordables_map.h"
#include "universal_data_logger.h"

namespace nest
{

/** @BeginDocumentation
Name: rate_neuron_opn - Base class for rate model with output noise.

Description:

Base class for rate model with output noise of the form

$\tau dX_i(t) / dt = - X_i(t) + \mu + \phi( \sum w_{ij} \cdot
                     \psi( X_j(t-d_{ij}) + \sqrt{tau} \cdot
                     \sigma \cdot \xi_j(t) ) )$

or

$\tau dX_i(t) / dt = - X_i(t) + \mu
                     + \text{mult\_coupling\_ex}( X_i(t) ) \cdot
                     \phi( \sum w^{ > 0 }_{ij} \cdot \psi( X_j(t-d_{ij})
                     + \sqrt{tau} \cdot \sigma \cdot \xi_j(t) ) )
                     + \text{mult\_coupling\_in}( X_i(t) ) \cdot
                     \phi( \sum w^{ < 0 }_{ij} \cdot \psi( X_j(t-d_{ij})
                     + \sqrt{tau} \cdot \sigma \cdot \xi_j(t) ) )$.

Here $\xi_j(t)$ denotes a Gaussian white noise.

This template class needs to be instantiated with a class
containing the following functions:
 - input (nonlinearity that is applied to the input, either psi or phi)
 - mult_coupling_ex (factor of multiplicative coupling for excitatory input)
 - mult_coupling_in (factor of multiplicative coupling for inhibitory input)

The boolean parameter linear_summation determines whether the input function
is applied to the summed up incoming connections (True, default value, input
represents phi) or to each input individually (False, input represents psi).
In case of multiplicative coupling the nonlinearity is applied separately
to the summed excitatory and inhibitory inputs if linear_summation=True.

References:

Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,
Bolten, M., Helias, M. and Diesmann, M. (2017).
Integration of Continuous-Time Dynamics in a
Spiking Neural Network Simulator.
Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034

Author: David Dahmen, Jan Hahne, Jannis Schuecker

SeeAlso: lin_rate, tanh_rate, threshold_lin_rate
 */
template < class TNonlinearities >
class rate_neuron_opn : public Archiving_Node
{

public:
  typedef Node base;

  rate_neuron_opn();
  rate_neuron_opn( const rate_neuron_opn& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::sends_secondary_event;

  void handle( InstantaneousRateConnectionEvent& );
  void handle( DelayedRateConnectionEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( InstantaneousRateConnectionEvent&, rport );
  port handles_test_event( DelayedRateConnectionEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void
  sends_secondary_event( InstantaneousRateConnectionEvent& )
  {
  }
  void
  sends_secondary_event( DelayedRateConnectionEvent& )
  {
  }

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  TNonlinearities nonlinearities_;

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long );
  bool wfr_update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< rate_neuron_opn< TNonlinearities > >;
  friend class UniversalDataLogger< rate_neuron_opn< TNonlinearities > >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {

    /** Time constant in ms. */
    double tau_;

    /** Noise parameter. */
    double sigma_;

    /** Mean input.*/
    double mu_;

    /** Target of non-linearity.
        True (default): Gain function applied to linearly summed input.
        False: Gain function applied to each input before summation.
    **/
    bool linear_summation_;

    /** use multiplicative coupling? Default is false */
    bool mult_coupling_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double rate_;       //!< Rate
    double noise_;      //!< Noise
    double noisy_rate_; //!< Noisy rate, i.e. rate +noise

    State_(); //!< Default initialization

    void get( DictionaryDatum& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( rate_neuron_opn& );
    Buffers_( const Buffers_&, rate_neuron_opn& );


    RingBuffer delayed_rates_ex_; //!< buffer for rate vector received by
    // RateConnectionDelayed from excitatory neurons
    RingBuffer delayed_rates_in_; //!< buffer for rate vector received by
    // RateConnectionDelayed from inhibitory neurons
    std::vector< double >
      instant_rates_ex_; //!< buffer for rate vector received
    // by RateConnectionInstantaneous from excitatory neurons
    std::vector< double >
      instant_rates_in_; //!< buffer for rate vector received
    // by RateConnectionInstantaneous
    std::vector< double >
      last_y_values; //!< remembers y_values from last wfr_update
    std::vector< double > random_numbers; //!< remembers the random_numbers in
    // order to apply the same random
    // numbers in each iteration when wfr
    // is used
    UniversalDataLogger< rate_neuron_opn >
      logger_; //!< Logger for all analog data
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    // propagators
    double P1_;
    double P2_;

    // factor accounting for piecewise constant implementation of noise
    double output_noise_factor_;

    librandom::RngPtr rng_;
    librandom::PoissonRandomDev poisson_dev_; //!< random deviate generator
    librandom::NormalRandomDev normal_dev_;   //!< random deviate generator
  };

  //! Read out the rate
  double
  get_rate_() const
  {
    return S_.rate_;
  }

  //! Read out the noise
  double
  get_noise_() const
  {
    return S_.noise_;
  }

  //! Read out the noisy rate
  double
  get_noisy_rate_() const
  {
    return S_.noisy_rate_;
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< rate_neuron_opn< TNonlinearities > > recordablesMap_;
};


template < class TNonlinearities >
inline void
rate_neuron_opn< TNonlinearities >::update( Time const& origin,
  const long from,
  const long to )
{
  update_( origin, from, to, false );
}

template < class TNonlinearities >
inline bool
rate_neuron_opn< TNonlinearities >::wfr_update( Time const& origin,
  const long from,
  const long to )
{
  State_ old_state = S_; // save state before wfr update
  const bool wfr_tol_exceeded = update_( origin, from, to, true );
  S_ = old_state; // restore old state

  return not wfr_tol_exceeded;
}

template < class TNonlinearities >
inline port
rate_neuron_opn< TNonlinearities >::handles_test_event(
  InstantaneousRateConnectionEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline port
rate_neuron_opn< TNonlinearities >::handles_test_event(
  DelayedRateConnectionEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline port
rate_neuron_opn< TNonlinearities >::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

template < class TNonlinearities >
inline void
rate_neuron_opn< TNonlinearities >::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  nonlinearities_.get( d );
}

template < class TNonlinearities >
inline void
rate_neuron_opn< TNonlinearities >::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

  nonlinearities_.set( d );
}

} // namespace

#endif /* #ifndef RATE_NEURON_OPN_H */
