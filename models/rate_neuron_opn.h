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

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <string>

// Includes from libnestutil:
#include "numerics.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "node.h"
#include "random_generators.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

namespace nest
{

/* BeginUserDocs: neuron, rate

Short description
+++++++++++++++++

Base class for rate model with output noise

Description
+++++++++++

Base class for rate model with output noise of the form

.. math::

 \tau dX_i(t) / dt = - X_i(t) + \mu + \phi( \sum w_{ij} \cdot
                     \psi( X_j(t-d_{ij}) + \sqrt{\tau} \cdot
                     \sigma \cdot \xi_j(t) ) )

or

.. math::

 \tau dX_i(t) / dt = - X_i(t) + \mu
                     + \text{mult_coupling_ex}( X_i(t) ) \cdot \\
                     \phi( \sum w^{ > 0 }_{ij} \cdot \psi( X_j(t-d_{ij}) \\
                     + \sqrt{\tau} \cdot \sigma \cdot \xi_j(t) ) ) \\
                     + \text{mult_coupling_in}( X_i(t) ) \cdot \\
                     \phi( \sum w^{ < 0 }_{ij} \cdot \psi( X_j(t-d_{ij}) \\
                     + \sqrt{\tau} \cdot \sigma \cdot \xi_j(t) ) )


Here :math:`xi_j(t)` denotes a Gaussian white noise.

This template class needs to be instantiated with a class
containing the following functions:

- ``input`` (nonlinearity that is applied to the input, either psi or phi)
- ``mult_coupling_ex`` (factor of multiplicative coupling for excitatory input)
- ``mult_coupling_in`` (factor of multiplicative coupling for inhibitory input)

The boolean parameter ``linear_summation`` determines whether the input function
is applied to the summed up incoming connections (True, default value, input
represents phi) or to each input individually (False, input represents psi).
In case of multiplicative coupling the nonlinearity is applied separately
to the summed excitatory and inhibitory inputs if ``linear_summation=True``.

See also  [1]_.

References
++++++++++

.. [1] Hahne J, Dahmen D, Schuecker J, Frommer A, Bolten M, Helias M,
       Diesmann M (2017). Integration of continuous-time dynamics in a
       spiking neural network simulator. Frontiers in Neuroinformatics, 11:34.
       DOI:  https://doi.org./10.3389/fninf.2017.00034

See also
++++++++

lin_rate, tanh_rate, threshold_lin_rate

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: rate_neuron_opn

EndUserDocs  */

template < class TNonlinearities >
class rate_neuron_opn : public ArchivingNode
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
  using Node::handles_test_event;
  using Node::sends_secondary_event;

  void handle( InstantaneousRateConnectionEvent& ) override;
  void handle( DelayedRateConnectionEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( InstantaneousRateConnectionEvent&, size_t ) override;
  size_t handles_test_event( DelayedRateConnectionEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void
  sends_secondary_event( InstantaneousRateConnectionEvent& ) override
  {
  }
  void
  sends_secondary_event( DelayedRateConnectionEvent& ) override
  {
  }

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  TNonlinearities nonlinearities_;

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long ) override;
  bool wfr_update( Time const&, const long, const long ) override;

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

    void set( const DictionaryDatum&, Node* node );
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
    void set( const DictionaryDatum&, Node* node );
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
    std::vector< double > instant_rates_ex_; //!< buffer for rate vector received
    // by RateConnectionInstantaneous from excitatory neurons
    std::vector< double > instant_rates_in_; //!< buffer for rate vector received
    // by RateConnectionInstantaneous
    std::vector< double > last_y_values;  //!< remembers y_values from last wfr_update
    std::vector< double > random_numbers; //!< remembers the random_numbers in
    // order to apply the same random numbers in each iteration when wfr is used
    UniversalDataLogger< rate_neuron_opn > logger_; //!< Logger for all analog data
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

    normal_distribution normal_dist_; //!< normal distribution
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
rate_neuron_opn< TNonlinearities >::update( Time const& origin, const long from, const long to )
{
  update_( origin, from, to, false );
}

template < class TNonlinearities >
inline bool
rate_neuron_opn< TNonlinearities >::wfr_update( Time const& origin, const long from, const long to )
{
  State_ old_state = S_; // save state before wfr update
  const bool wfr_tol_exceeded = update_( origin, from, to, true );
  S_ = old_state; // restore old state

  return not wfr_tol_exceeded;
}

template < class TNonlinearities >
inline size_t
rate_neuron_opn< TNonlinearities >::handles_test_event( InstantaneousRateConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline size_t
rate_neuron_opn< TNonlinearities >::handles_test_event( DelayedRateConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline size_t
rate_neuron_opn< TNonlinearities >::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
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
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  nonlinearities_.get( d );
}

template < class TNonlinearities >
inline void
rate_neuron_opn< TNonlinearities >::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, this );   // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

  nonlinearities_.set( d, this );
}

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
RecordablesMap< rate_neuron_opn< TNonlinearities > > rate_neuron_opn< TNonlinearities >::recordablesMap_;


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
nest::rate_neuron_opn< TNonlinearities >::Parameters_::Parameters_()
  : tau_( 10.0 ) // ms
  , sigma_( 1.0 )
  , mu_( 0.0 )
  , linear_summation_( true )
  , mult_coupling_( false )
{
  recordablesMap_.create();
}

template < class TNonlinearities >
nest::rate_neuron_opn< TNonlinearities >::State_::State_()
  : rate_( 0.0 )
  , noise_( 0.0 )
  , noisy_rate_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::tau, tau_ );
  def< double >( d, names::sigma, sigma_ );
  def< double >( d, names::mu, mu_ );
  def< bool >( d, names::linear_summation, linear_summation_ );
  def< bool >( d, names::mult_coupling, mult_coupling_ );

  // Also allow old names (to not break old scripts)
  def< double >( d, names::std, sigma_ );
  def< double >( d, names::mean, mu_ );
}

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::tau, tau_, node );
  updateValueParam< double >( d, names::mu, mu_, node );
  updateValueParam< double >( d, names::sigma, sigma_, node );
  updateValueParam< bool >( d, names::linear_summation, linear_summation_, node );
  updateValueParam< bool >( d, names::mult_coupling, mult_coupling_, node );

  // Check for old names
  if ( updateValueParam< double >( d, names::mean, mu_, node ) )
  {
    LOG( M_WARNING,
      "rate_neuron_opn< TNonlinearities >::Parameters_::set",
      "The parameter mean has been renamed to mu. Please use the new "
      "name from now on." );
  }

  if ( updateValueParam< double >( d, names::std, sigma_, node ) )
  {
    LOG( M_WARNING,
      "rate_neuron_opn< TNonlinearities >::Parameters_::set",
      "The parameter std has been renamed to sigma. Please use the new "
      "name from now on." );
  }

  // Check for invalid parameters
  if ( tau_ <= 0 )
  {
    throw BadProperty( "Time constant must be > 0." );
  }
  if ( sigma_ < 0 )
  {
    throw BadProperty( "Noise parameter must not be negative." );
  }
}

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, rate_ );             // Rate
  def< double >( d, names::noise, noise_ );           // Noise
  def< double >( d, names::noisy_rate, noisy_rate_ ); // Noisy rate
}

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::State_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::rate, rate_, node ); // Rate
}

template < class TNonlinearities >
nest::rate_neuron_opn< TNonlinearities >::Buffers_::Buffers_( rate_neuron_opn< TNonlinearities >& n )
  : logger_( n )
{
}

template < class TNonlinearities >
nest::rate_neuron_opn< TNonlinearities >::Buffers_::Buffers_( const Buffers_&, rate_neuron_opn< TNonlinearities >& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
nest::rate_neuron_opn< TNonlinearities >::rate_neuron_opn()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  Node::set_node_uses_wfr( kernel::manager< SimulationManager >.use_wfr() );
}

template < class TNonlinearities >
nest::rate_neuron_opn< TNonlinearities >::rate_neuron_opn( const rate_neuron_opn& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  Node::set_node_uses_wfr( kernel::manager< SimulationManager >.use_wfr() );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::init_buffers_()
{
  B_.delayed_rates_ex_.clear(); // includes resize
  B_.delayed_rates_in_.clear(); // includes resize

  // resize buffers
  const size_t buffer_size = kernel::manager< ConnectionManager >.get_min_delay();
  B_.instant_rates_ex_.resize( buffer_size, 0.0 );
  B_.instant_rates_in_.resize( buffer_size, 0.0 );
  B_.last_y_values.resize( buffer_size, 0.0 );
  B_.random_numbers.resize( buffer_size, numerics::nan );

  // initialize random numbers
  for ( unsigned int i = 0; i < buffer_size; i++ )
  {
    B_.random_numbers[ i ] = V_.normal_dist_( get_vp_specific_rng( get_thread() ) );
  }

  B_.logger_.reset(); // includes resize
  ArchivingNode::clear_history();
}

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  const double h = Time::get_resolution().get_ms();

  // propagators
  V_.P1_ = std::exp( -h / P_.tau_ );
  V_.P2_ = -numerics::expm1( -h / P_.tau_ );

  // Gaussian white noise approximated by piecewise constant value
  V_.output_noise_factor_ = std::sqrt( P_.tau_ / h );
}

/* ----------------------------------------------------------------
 * Update and event handling functions
 */

template < class TNonlinearities >
bool
nest::rate_neuron_opn< TNonlinearities >::update_( Time const& origin,
  const long from,
  const long to,
  const bool called_from_wfr_update )
{
  const size_t buffer_size = kernel::manager< ConnectionManager >.get_min_delay();
  const double wfr_tol = kernel::manager< SimulationManager >.get_wfr_tol();
  bool wfr_tol_exceeded = false;

  // allocate memory to store rates to be sent by rate events
  std::vector< double > new_rates( buffer_size, 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    // get noise
    S_.noise_ = P_.sigma_ * B_.random_numbers[ lag ];
    // the noise is added to the noisy_rate variable
    S_.noisy_rate_ = S_.rate_ + V_.output_noise_factor_ * S_.noise_;
    // store rate
    new_rates[ lag ] = S_.noisy_rate_;
    // propagate rate to new time step (exponential integration)
    S_.rate_ = V_.P1_ * S_.rate_ + V_.P2_ * P_.mu_;

    double delayed_rates_in = 0;
    double delayed_rates_ex = 0;
    if ( called_from_wfr_update )
    {
      // use get_value_wfr_update to keep values in buffer
      delayed_rates_in = B_.delayed_rates_in_.get_value_wfr_update( lag );
      delayed_rates_ex = B_.delayed_rates_ex_.get_value_wfr_update( lag );
    }
    else
    {
      // use get_value to clear values in buffer after reading
      delayed_rates_in = B_.delayed_rates_in_.get_value( lag );
      delayed_rates_ex = B_.delayed_rates_ex_.get_value( lag );
    }
    double instant_rates_in = B_.instant_rates_in_[ lag ];
    double instant_rates_ex = B_.instant_rates_ex_[ lag ];
    double H_ex = 1.; // valid value for non-multiplicative coupling
    double H_in = 1.; // valid value for non-multiplicative coupling
    if ( P_.mult_coupling_ )
    {
      H_ex = nonlinearities_.mult_coupling_ex( new_rates[ lag ] );
      H_in = nonlinearities_.mult_coupling_in( new_rates[ lag ] );
    }

    if ( P_.linear_summation_ )
    {
      // In this case we explicitly need to distinguish the cases of
      // multiplicative coupling and non-multiplicative coupling in
      // order to compute input( ex + in ) instead of input(ex) + input(in) in
      // the non-multiplicative case.
      if ( P_.mult_coupling_ )
      {
        S_.rate_ += V_.P2_ * H_ex * nonlinearities_.input( delayed_rates_ex + instant_rates_ex );
        S_.rate_ += V_.P2_ * H_in * nonlinearities_.input( delayed_rates_in + instant_rates_in );
      }
      else
      {
        S_.rate_ +=
          V_.P2_ * nonlinearities_.input( delayed_rates_ex + instant_rates_ex + delayed_rates_in + instant_rates_in );
      }
    }
    else
    {
      // In this case multiplicative and non-multiplicative coupling
      // can be handled with the same code.
      S_.rate_ += V_.P2_ * H_ex * ( delayed_rates_ex + instant_rates_ex );
      S_.rate_ += V_.P2_ * H_in * ( delayed_rates_in + instant_rates_in );
    }

    if ( called_from_wfr_update )
    {
      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded or fabs( S_.rate_ - B_.last_y_values[ lag ] ) > wfr_tol;
      // update last_y_values for next wfr iteration
      B_.last_y_values[ lag ] = S_.rate_;
    }
    else
    {
      // rate logging
      B_.logger_.record_data( origin.get_steps() + lag );
    }
  }

  if ( not called_from_wfr_update )
  {
    // Send delay-rate-neuron-event. This only happens in the final iteration
    // to avoid accumulation in the buffers of the receiving neurons.
    DelayedRateConnectionEvent drve;
    drve.set_coeffarray( new_rates );
    kernel::manager< EventDeliveryManager >.send_secondary( *this, drve );

    // clear last_y_values
    std::vector< double >( buffer_size, 0.0 ).swap( B_.last_y_values );

    // modify new_rates for rate-neuron-event as proxy for next min_delay
    for ( long temp = from; temp < to; ++temp )
    {
      new_rates[ temp ] = S_.noisy_rate_;
    }

    // create new random numbers
    B_.random_numbers.resize( buffer_size, numerics::nan );
    for ( unsigned int i = 0; i < buffer_size; i++ )
    {
      B_.random_numbers[ i ] = V_.normal_dist_( get_vp_specific_rng( get_thread() ) );
    }
  }

  // Send rate-neuron-event
  InstantaneousRateConnectionEvent rve;
  rve.set_coeffarray( new_rates );
  kernel::manager< EventDeliveryManager >.send_secondary( *this, rve );

  // Reset variables
  std::vector< double >( buffer_size, 0.0 ).swap( B_.instant_rates_ex_ );
  std::vector< double >( buffer_size, 0.0 ).swap( B_.instant_rates_in_ );

  return wfr_tol_exceeded;
}


template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::handle( InstantaneousRateConnectionEvent& e )
{
  const double weight = e.get_weight();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      if ( weight >= 0.0 )
      {
        B_.instant_rates_ex_[ i ] += weight * e.get_coeffvalue( it );
      }
      else
      {
        B_.instant_rates_in_[ i ] += weight * e.get_coeffvalue( it );
      }
    }
    else
    {
      if ( weight >= 0.0 )
      {
        B_.instant_rates_ex_[ i ] += weight * nonlinearities_.input( e.get_coeffvalue( it ) );
      }
      else
      {
        B_.instant_rates_in_[ i ] += weight * nonlinearities_.input( e.get_coeffvalue( it ) );
      }
    }
    i++;
  }
}

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::handle( DelayedRateConnectionEvent& e )
{
  const double weight = e.get_weight();
  const long delay = e.get_delay_steps() - kernel::manager< ConnectionManager >.get_min_delay();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      if ( weight >= 0.0 )
      {
        B_.delayed_rates_ex_.add_value( delay + i, weight * e.get_coeffvalue( it ) );
      }
      else
      {
        B_.delayed_rates_in_.add_value( delay + i, weight * e.get_coeffvalue( it ) );
      }
    }
    else
    {
      if ( weight >= 0.0 )
      {
        B_.delayed_rates_ex_.add_value( delay + i, weight * nonlinearities_.input( e.get_coeffvalue( it ) ) );
      }
      else
      {
        B_.delayed_rates_in_.add_value( delay + i, weight * nonlinearities_.input( e.get_coeffvalue( it ) ) );
      }
    }
    ++i;
  }
}

template < class TNonlinearities >
void
nest::rate_neuron_opn< TNonlinearities >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace

#endif /* #ifndef RATE_NEURON_OPN_H */
