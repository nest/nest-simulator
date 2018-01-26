/*
 *  parrot_rate_neuron.h
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

#ifndef PARROT_RATE_NEURON_H
#define PARROT_RATE_NEURON_H

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

/* BeginDocumentation
Name: parrot_rate_neuron - Rate neuron that sums up incoming rates and applies
                           a nonlinearity specified via the template.

Description:

The parrot rate neuron simply sums up all incoming rates and applies
the nonlinearity specified in the function input of the template class.
An important application is to provide the possibility to
apply different nonlinearities to different incoming connections of the
same rate neuron by connecting the sending rate neurons to the
parrot rate neuron and connecting the parrot rate neuron to the
receiving rate neuron instead of using a direct connection.
Please note that for instantaneous rate connections the rate arrives
one time step later at the receiving rate neurons as with a direct connection.

Remarks:

- Weights on connections from and to the parrot_rate_neuron
  are handled as usual.
- Delays are honored on incoming and outgoing connections.

Receives: InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Sends: InstantaneousRateConnectionEvent, DelayedRateConnectionEvent

Parameters:
All Parameters from the class Nonlinearities can be set in the
status dictionary.

Author: Mario Senden, Jan Hahne, Jannis Schuecker
FirstVersion: November 2017
*/
template < class TNonlinearities >
class parrot_rate_neuron : public Archiving_Node
{

public:
  typedef Node base;

  parrot_rate_neuron();
  parrot_rate_neuron( const parrot_rate_neuron& );

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

  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long );
  bool wfr_update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< parrot_rate_neuron< TNonlinearities > >;
  friend class UniversalDataLogger< parrot_rate_neuron< TNonlinearities > >;


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
    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( parrot_rate_neuron& );
    Buffers_( const Buffers_&, parrot_rate_neuron& );

    // buffer for rate vector received by DelayRateConnection
    RingBuffer delayed_rates_;

    // buffer for rate vector received by RateConnection
    std::vector< double > instant_rates_;

    // remembers y_values from last wfr_update
    std::vector< double > last_y_values;

    //! Logger for all analog data
    UniversalDataLogger< parrot_rate_neuron > logger_;
  };

  // ----------------------------------------------------------------


  //! Read out the rate
  double
  get_rate_() const
  {
    return S_.rate_;
  }

  // ----------------------------------------------------------------

  State_ S_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< parrot_rate_neuron< TNonlinearities > >
    recordablesMap_;
};

template < class TNonlinearities >
inline void
parrot_rate_neuron< TNonlinearities >::update( Time const& origin,
  const long from,
  const long to )
{
  update_( origin, from, to, false );
}

template < class TNonlinearities >
inline bool
parrot_rate_neuron< TNonlinearities >::wfr_update( Time const& origin,
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
parrot_rate_neuron< TNonlinearities >::handles_test_event(
  InstantaneousRateConnectionEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

template < class TNonlinearities >
inline port
parrot_rate_neuron< TNonlinearities >::handles_test_event(
  DelayedRateConnectionEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

template < class TNonlinearities >
inline port
parrot_rate_neuron< TNonlinearities >::handles_test_event(
  DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

template < class TNonlinearities >
inline void
parrot_rate_neuron< TNonlinearities >::get_status( DictionaryDatum& d ) const
{
  S_.get( d );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  nonlinearities_.get( d );
}

template < class TNonlinearities >
inline void
parrot_rate_neuron< TNonlinearities >::set_status( const DictionaryDatum& d )
{
  State_ stmp = S_; // temporary copy in case of errors
  stmp.set( d );    // throws if BadProperty

  // We now know that (stmp) is consistent. We do not
  // write it back to (S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  S_ = stmp;

  nonlinearities_.set( d );
}

} // namespace

#endif /* #ifndef PARROT_RATE_NEURON_H */
