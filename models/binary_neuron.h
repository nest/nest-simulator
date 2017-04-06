/*
 *  binary_neuron.h
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

#ifndef BINARY_NEURON_H
#define BINARY_NEURON_H

// C++ includes:
#include <cmath>

// Includes from librandom:
#include "exp_randomdev.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{
/**
 * Binary stochastic neuron with linear or sigmoidal gain function.
 *
 * This class is a base class that needs to be instantiated with a gain
 * function.
 *
 * @see ginzburg_neuron, mccullogh_pitts_neuron
 */
template < class TGainfunction >
class binary_neuron : public Archiving_Node
{

public:
  binary_neuron();
  binary_neuron( const binary_neuron& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;
  using Node::receives_signal;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  SignalType sends_signal() const;
  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );


private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  // gain function functor
  // must have an double operator(double) defined
  TGainfunction gain_;

  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< binary_neuron< TGainfunction > >;
  friend class UniversalDataLogger< binary_neuron< TGainfunction > >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    //! mean inter-update interval in ms (acts like a membrane time constant).
    double tau_m_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    bool y_;               //!< output of neuron in [0,1]
    double h_;             //!< total input current to neuron
    double last_in_gid_;   //!< gid of the last spike being received
    Time t_next_;          //!< time point of next update
    Time t_last_in_spike_; //!< time point of last input spike seen

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
    Buffers_( binary_neuron& );
    Buffers_( const Buffers_&, binary_neuron& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;


    //! Logger for all analog data
    UniversalDataLogger< binary_neuron > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    librandom::RngPtr rng_; //!< random number generator of my own thread
    librandom::ExpRandomDev exp_dev_; //!< random deviate generator
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the binary_neuron state of the neuron
  double
  get_output_state__() const
  {
    return S_.y_;
  }

  //! Read out the summed input of the neuron (= membrane potential)
  double
  get_input__() const
  {
    return S_.h_;
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
  static RecordablesMap< binary_neuron< TGainfunction > > recordablesMap_;
};


template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::handles_test_event( SpikeEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::handles_test_event( CurrentEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


template < class TGainfunction >
inline SignalType
binary_neuron< TGainfunction >::sends_signal() const
{
  return BINARY;
}

template < class TGainfunction >
inline SignalType
binary_neuron< TGainfunction >::receives_signal() const
{
  return BINARY;
}


template < class TGainfunction >
inline void
binary_neuron< TGainfunction >::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  gain_.get( d );
}

template < class TGainfunction >
inline void
binary_neuron< TGainfunction >::set_status( const DictionaryDatum& d )
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

  gain_.set( d );
}

} // namespace

#endif /* #ifndef BINARY_NEURON_H */
