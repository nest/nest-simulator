/*
 *  sli_neuron.h
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

#ifndef SLI_NEURON_H
#define SLI_NEURON_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/** @BeginDocumentation
Name: sli_neuron - neuron with SLI callback

Description:
The sli_neuron is a model whose state, update and calibration can be
defined in SLI.

The state of the neuron is a SLI dictionary which can be retrieved
with GetStatus. The state should contain two procedures:
1. /update
2. /calibrate

/calibrate is called before the simulation starts. It is used to
         pre-compute constants of the dynamics and to scale
         parameters to the temporal resolution.  A minimal
         implementation of calibrate is:

         /calibrate { GetResolution /h Set } def

/update    is called during simulation and must propagate the node's
         state by one integration step h.  If /update decides that
         the node should spike, it must set the variable /spike to
         true.

Both /calibrate and /update are called with the node's status
dictionary on top of the dictionary stack.  This means that node
variables override names in the systemdict or in the userdict.
Moreover, all definitions are done in the node's statusdict and
persist throughout the simulation.

Errors.
If an error occurs during the evaluation of /calibrate or /update, the
errorneous neuron is skipped and update proceeds to the next node
until the time-slice is finished. After the time-slice is finished,
simulation terminates with an additional error message that issues the
global id of the errorneous node.

Errors are handled by the SLI standard errorhandler. In addition, the
contents of the error dictionary is copied into the node's status
dictionary, to allow debugging of the node.

Parameters:

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

Author: Diesmann, Plesser, Gewaltig

FirstVersion: January 2009

SeeAlso: iaf_psc_delta, iaf_psc_exp, iaf_cond_exp, testsuite::test_sli_neuron
*/

/**
 *  neuron with state and dynamics defined as SLI code
 */
class sli_neuron : public Archiving_Node
{

public:
  sli_neuron();
  sli_neuron( const sli_neuron& );

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
  DictionaryDatum get_status_dict_();

  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  /**
   * Execute a SLI command in the neuron's namespace.
   */
  int execute_sli_protected( DictionaryDatum, Name );

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< sli_neuron >;
  friend class UniversalDataLogger< sli_neuron >;

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( sli_neuron& );
    Buffers_( const Buffers_&, sli_neuron& );

    /** buffers and summs up incoming spikes/currents */
    RingBuffer ex_spikes_;
    RingBuffer in_spikes_;
    RingBuffer currents_;


    /** Logger for all analog data. */
    UniversalDataLogger< sli_neuron > logger_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    double vm = 0.0;
    if ( state_->known( names::V_m ) )
    {
      vm = getValue< double >( state_, names::V_m );
    }

    return vm;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup sli_neuron
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  DictionaryDatum state_;
  /**
   * These are pointers into the status dictionary and must be updated in
   * calibrate.
   */
  Token* vm_t;         //!< Points to V_m
  Token* update_t;     //!< points to update
  Token* calibrate_t;  //!< points to calibrate
  Token* in_spikes_t;  //!< number of excitatory spikes during the time slice
  Token* ex_spikes_t;  //!< number of inhibitory spikes during the sime slice
  Token* currents_t;   //!< external current
  Token* last_spike_t; //!< time of last spike
  Token* out_events_t; //!< Events produced by the neuron
  Token* spike_t;      //< Boolean for fast spike initiation
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< sli_neuron > recordablesMap_;

  /** @} */
};

inline port
sli_neuron::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
sli_neuron::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
sli_neuron::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
sli_neuron::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


inline DictionaryDatum
sli_neuron::get_status_dict_()
{
  return state_;
}

inline void
sli_neuron::get_status( DictionaryDatum& d ) const
{
  // We needn't do anything else here, since d already points to
  // sli_neuron::state_, because of Node::get_status_dict_().
  //
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
sli_neuron::set_status( const DictionaryDatum& d )
{
  Archiving_Node::set_status( d );

  // To initialize the state dictionary, we copy all entries from d into s.
  // Later, the state dictionary will be in the interpreter and values are
  // changed automatically. SetStatus is then only needed to change properties
  // of Archiving_Node.
  for ( TokenMap::const_iterator it = d->begin(); it != d->end(); ++it )
  {
    state_->insert( it->first, it->second );
    it->second.set_access_flag();
  }
}

} // namespace

#endif /* #ifndef SLI_NEURON_H */
