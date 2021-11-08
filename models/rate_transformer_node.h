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
#include "universal_data_logger.h"

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
parameter linear_summation determines whether the input function is applied to
the summed up incoming connections (True, default value, input
represents phi) or to each input individually (False, input represents psi).

An important application is to provide the possibility to
apply different nonlinearities to different incoming connections of the
same rate neuron by connecting the sending rate neurons to the
rate transformer node and connecting the rate transformer node to the
receiving rate neuron instead of using a direct connection.
Please note that for instantaneous rate connections the rate arrives
one time step later at the receiving rate neurons as with a direct connection.

Remarks:

- Weights on connections from and to the rate_transformer_node
  are handled as usual.
- Delays are honored on incoming and outgoing connections.

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
  using Node::sends_secondary_event;
  using Node::handles_test_event;

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
  void init_buffers_();
  void calibrate();

  TNonlinearities nonlinearities_;

  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long );
  bool wfr_update( Time const&, const long, const long );

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
inline port
rate_transformer_node< TNonlinearities >::handles_test_event( InstantaneousRateConnectionEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline port
rate_transformer_node< TNonlinearities >::handles_test_event( DelayedRateConnectionEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TNonlinearities >
inline port
rate_transformer_node< TNonlinearities >::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
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

} // namespace

#endif /* #ifndef RATE_TRANSFORMER_NODE_H */
