/*
 *  iaf_neat.h
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

#ifndef IAF_NEAT_H
#define IAF_NEAT_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

#include "compartment_tree_neat.h"
#include "synapses_neat.h"

namespace nest
{

/* BeginUserDocs: neuron

Short description
+++++++++++++++++

A neuron model with user-defined structure and AMPA, GABA or NMDA
receptors.

Description
+++++++++++

iaf_neat is an implementation of a leaky-integrator neuron. Users can
define the structure of the neuron, i.e., soma and dendritic tree by
adding compartments. Each compartment can be assigned receptors,
currently modeled by AMPA, GABA or NMDA dynamics. <add info about
spiking/refractory implementation><add info about recording>

Usage
+++++
<add info about adding compartsments, receptors>
<add info about recording>
<add info about currents>

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=========== ======= ===========================================================
 V_th       mV      Spike threshold
=========== ======= ===========================================================

<add info about setting compartment, receptor parameters>

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

<add reference?>

See also
++++++++

There's nothing like it. ^^

EndUserDocs*/

class iaf_neat : public Archiving_Node
{

public:
  iaf_neat();
  iaf_neat( const iaf_neat& );

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

  void add_compartment( const long compartment_idx, const long parent_compartment_idx, const DictionaryDatum& compartment_params ) override;
  size_t add_receptor( const long compartment_idx, const std::string& type ) override;

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  CompTree m_c_tree;

  std::vector< std::shared_ptr< Synapse > > syn_receptors;

  // To record variables with DataAccessFunctor
  double get_state_element( size_t elem){return m_c_tree.get_node_voltage(elem);}

  // The next classes need to be friends to access the State_ class/member
  friend class DataAccessFunctor< iaf_neat >;
  friend class DynamicRecordablesMap< iaf_neat >;
  friend class DynamicUniversalDataLogger< iaf_neat >;

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_neat& );
    Buffers_( const Buffers_&, iaf_neat& );

    /** buffers and summs up incoming spikes/currents */
    RingBuffer currents_;

    //! Logger for all analog data
    DynamicUniversalDataLogger< iaf_neat > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_alpha_data
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Buffers_ B_;
  /** @} */

  //! Mapping of recordables names to access functions
  DynamicRecordablesMap< iaf_neat > recordablesMap_;

  double V_th_;
};


inline port
nest::iaf_neat::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_neat::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( ( receptor_type < 0 ) or ( receptor_type >= static_cast< port >( syn_receptors.size() ) ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }
  return receptor_type;
}

inline port
iaf_neat::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_neat::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_neat::get_status( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_neat::set_status( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_th, V_th_ );
  Archiving_Node::set_status( d );
}

} // namespace

#endif /* #ifndef IAF_NEAT_H */
