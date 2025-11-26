/*
 *  rate_transformer_node.h
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

#ifndef RATE_TRANSFORMER_NODE_H
#define RATE_TRANSFORMER_NODE_H

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
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

namespace nest
{

/* BeginUserDocs: neuron, rate

Short description
+++++++++++++++++

Rate neuron that sums up incoming rates and applies a nonlinearity specified via the template

Description
+++++++++++

Base class for rate transformer model of the form

.. math::

   X_i(t) = \phi( \sum w_{ij} \cdot \psi( X_j(t-d_{ij}) ) )

The rate transformer node simply applies the nonlinearity specified in the
input-function of the template class to all incoming inputs. The boolean
parameter ``linear_summation`` determines whether the input function is applied to
the summed up incoming connections (True, default value, input
represents phi) or to each input individually (False, input represents psi).

An important application is to provide the possibility to
apply different nonlinearities to different incoming connections of the
same rate neuron by connecting the sending rate neurons to the
rate transformer node and connecting the rate transformer node to the
receiving rate neuron instead of using a direct connection.
Please note that for instantaneous rate connections the rate arrives
one time step later at the receiving rate neurons as with a direct connection.

Weights on connections from and to the ``rate_transformer_node`` are
handled as usual. Delays are honored on incoming and outgoing
connections.

Receives
++++++++

InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Sends
+++++

InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Parameters
++++++++++

Only the parameter ``linear_summation`` and the parameters from the class ``Nonlinearities`` can be set in the
status dictionary.

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: rate_transformer_node

EndUserDocs */

template < class TNonlinearities >
class rate_transformer_node : public ArchivingNode
{

public:
  typedef Node base;

  rate_transformer_node();
  rate_transformer_node( const rate_transformer_node& );

  /**
   * Import sets of overloaded virtual functions.
   * We need to explicitly include sets of overloaded
   * virtual functions into the current scope.
   * According to the SUN C++ FAQ, this is the correct
   * way of doing things, although all other compilers
   * happily live without.
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

  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long ) override;
  bool wfr_update( Time const&, const long, const long ) override;

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< rate_transformer_node< TNonlinearities > >;
  friend class UniversalDataLogger< rate_transformer_node< TNonlinearities > >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Target of non-linearity.
        True (default): Gain function applied to linearly summed input.
        False: Gain function applied to each input before summation.
    **/
    bool linear_summation_;

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
    double rate_; //!< Rate

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
    Buffers_( rate_transformer_node& );
    Buffers_( const Buffers_&, rate_transformer_node& );

    // buffer for rate vector received by DelayRateConnection
    RingBuffer delayed_rates_;

    // buffer for rate vector received by RateConnection
    std::vector< double > instant_rates_;

    // remembers y_values from last wfr_update
    std::vector< double > last_y_values;

    //! Logger for all analog data
    UniversalDataLogger< rate_transformer_node > logger_;
  };

  // ----------------------------------------------------------------


  //! Read out the rate
  double
  get_rate_() const
  {
    return S_.rate_;
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< rate_transformer_node< TNonlinearities > > recordablesMap_;
};

template < class TNonlinearities >
inline void
rate_transformer_node< TNonlinearities >::update( Time const& origin, const long from, const long to )
{
  update_( origin, from, to, false );
}

template < class TNonlinearities >
inline bool
rate_transformer_node< TNonlinearities >::wfr_update( Time const& origin, const long from, const long to )
{
  State_ old_state = S_; // save state before wfr update
  const bool wfr_tol_exceeded = update_( origin, from, to, true );
  S_ = old_state; // restore old state

  return not wfr_tol_exceeded;
}

template < class TNonlinearities >
inline size_t
rate_transformer_node< TNonlinearities >::handles_test_event( InstantaneousRateConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline size_t
rate_transformer_node< TNonlinearities >::handles_test_event( DelayedRateConnectionEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline size_t
rate_transformer_node< TNonlinearities >::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

template < class TNonlinearities >
inline void
rate_transformer_node< TNonlinearities >::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  nonlinearities_.get( d );
}

template < class TNonlinearities >
inline void
rate_transformer_node< TNonlinearities >::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, this );   // throws if BadProperty

  // We now know that (stmp) is consistent. We do not
  // write it back to (S_) before we are also sure that
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
RecordablesMap< rate_transformer_node< TNonlinearities > > rate_transformer_node< TNonlinearities >::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::Parameters_::Parameters_()
  : linear_summation_( true )
{
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::State_::State_()
  : rate_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::Parameters_::get( DictionaryDatum& d ) const
{
  def< bool >( d, names::linear_summation, linear_summation_ );
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< bool >( d, names::linear_summation, linear_summation_, node );
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::rate, rate_ ); // Rate
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::State_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::rate, rate_, node ); // Rate
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::Buffers_::Buffers_( rate_transformer_node< TNonlinearities >& n )
  : logger_( n )
{
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::Buffers_::Buffers_( const Buffers_&,
  rate_transformer_node< TNonlinearities >& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::rate_transformer_node()
  : ArchivingNode()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  Node::set_node_uses_wfr( kernel::manager< SimulationManager >.use_wfr() );
}

template < class TNonlinearities >
nest::rate_transformer_node< TNonlinearities >::rate_transformer_node( const rate_transformer_node& n )
  : ArchivingNode( n )
  , nonlinearities_( n.nonlinearities_ )
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
nest::rate_transformer_node< TNonlinearities >::init_buffers_()
{
  B_.delayed_rates_.clear(); // includes resize

  // resize buffers
  const size_t buffer_size = kernel::manager< ConnectionManager >.get_min_delay();
  B_.instant_rates_.resize( buffer_size, 0.0 );
  B_.last_y_values.resize( buffer_size, 0.0 );

  B_.logger_.reset(); // includes resize
  ArchivingNode::clear_history();
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::pre_run_hook()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate
}

/* ----------------------------------------------------------------
 * Update and event handling functions
 */

template < class TNonlinearities >
bool
nest::rate_transformer_node< TNonlinearities >::update_( Time const& origin,
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
    // store rate
    new_rates[ lag ] = S_.rate_;
    // reinitialize output rate
    S_.rate_ = 0.0;

    double delayed_rates = 0;
    if ( called_from_wfr_update )
    {
      // use get_value_wfr_update to keep values in buffer
      delayed_rates = B_.delayed_rates_.get_value_wfr_update( lag );
    }
    else
    {
      // use get_value to clear values in buffer after reading
      delayed_rates = B_.delayed_rates_.get_value( lag );
    }

    if ( P_.linear_summation_ )
    {
      S_.rate_ += nonlinearities_.input( delayed_rates + B_.instant_rates_[ lag ] );
    }
    else
    {
      S_.rate_ += delayed_rates + B_.instant_rates_[ lag ];
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

    // modifiy new_rates for rate-neuron-event as proxy for next min_delay
    for ( long temp = from; temp < to; ++temp )
    {
      new_rates[ temp ] = S_.rate_;
    }
  }

  // Send rate-neuron-event
  InstantaneousRateConnectionEvent rve;
  rve.set_coeffarray( new_rates );
  kernel::manager< EventDeliveryManager >.send_secondary( *this, rve );

  // Reset variables
  std::vector< double >( buffer_size, 0.0 ).swap( B_.instant_rates_ );

  return wfr_tol_exceeded;
}


template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::handle( InstantaneousRateConnectionEvent& e )
{
  const double weight = e.get_weight();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    if ( P_.linear_summation_ )
    {
      B_.instant_rates_[ i ] += weight * e.get_coeffvalue( it );
    }
    else
    {
      B_.instant_rates_[ i ] += weight * nonlinearities_.input( e.get_coeffvalue( it ) );
    }
    ++i;
  }
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::handle( DelayedRateConnectionEvent& e )
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
      B_.delayed_rates_.add_value( delay + i, weight * e.get_coeffvalue( it ) );
    }
    else
    {
      B_.delayed_rates_.add_value( delay + i, weight * nonlinearities_.input( e.get_coeffvalue( it ) ) );
    }
    ++i;
  }
}

template < class TNonlinearities >
void
nest::rate_transformer_node< TNonlinearities >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace

#endif /* #ifndef RATE_TRANSFORMER_NODE_H */
