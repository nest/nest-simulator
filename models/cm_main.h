/*
 *  cm_main.h
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
#include "event.h"
#include "nest_types.h"
#include "universal_data_logger.h"

#include "cm_tree.h"
#include "cm_syns.h"

namespace nest
{

/* BeginUserDocs: neuron

Short description
+++++++++++++++++

A neuron model with user-defined dendrite structure.
Currently, AMPA, GABA or AMPA+NMDA receptors.

Description
+++++++++++

`cm_main` is an implementation of a compartmental model. Users can
define the structure of the neuron, i.e., soma and dendritic tree by
adding compartments. Each compartment can be assigned receptors,
currently modeled by AMPA, GABA or NMDA dynamics.

The default model is passive, but sodium and potassium currents can be added
by passing non-zero conductances 'g_Na' and 'g_K' with the parameter dictionary
when adding compartments. We are working on the inclusion of general ion channel
currents through NESTML.

Usage
+++++
The structure of the dendrite is user defined. Thus after creation of the neuron
in the standard manner

>>> cm =  nest.Create('cm_main')

users add compartments using the `nest.add_compartment()` function

>>> comp = nest.AddCompartment(cm, [compartment index], [parent index],
>>>                                [dictionary with compartment params])

After all compartments have been added, users can add receptors

>>> recept = nest.AddReceptor(cm, [compartment index], ['AMPA', 'GABA' or 'AMPA+NMDA'])

Compartment voltages can be recorded. To do so, users create a multimeter in the
standard manner but specify the to be recorded voltages as
'V_m_[compartment_index]', i.e.

>>> mm = nest.Create('multimeter', 1, {'record_from': ['V_m_[compartment_index]'], ...})

Current generators can be connected to the model. In this case, the receptor
type is the [compartment index], i.e.

>>> dc = nest.Create('dc_generator', {...})
>>> nest.Connect(dc, cm, syn_spec={..., 'receptor_type': [compartment index]}

Parameters
++++++++++

The following parameters can be set in the status dictionary.

=========== ======= ===========================================================
 V_th       mV      Spike threshold (default: -55.0mV)
=========== ======= ===========================================================

The following parameters can be set using the `AddCompartment` function

=========== ======= ===========================================================
 C_m        uF      Capacitance of compartment
 g_c        uS      Coupling conductance with parent compartment
 g_L        uS      Leak conductance of the compartment
 e_L        mV      Leak reversal of the compartment
=========== ======= ===========================================================

Receptor types for the moment are hardcoded. The choice is from
'AMPA', 'GABA' or 'NMDA'.

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

NEURON simulator ;-D

EndUserDocs*/

class cm_main : public Archiving_Node
{

public:
  cm_main();
  cm_main( const cm_main& );

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

  CompTree c_tree_;
  std::vector< std::shared_ptr< Synapse > > syn_receptors_;

  // To record variables with DataAccessFunctor
  double get_state_element( size_t elem){return c_tree_.get_compartment_voltage(elem);}

  // The next classes need to be friends to access the State_ class/member
  friend class DataAccessFunctor< cm_main >;
  friend class DynamicRecordablesMap< cm_main >;
  friend class DynamicUniversalDataLogger< cm_main >;

  //! Mapping of recordables names to access functions
  DynamicRecordablesMap< cm_main > recordablesMap_;
  //! Logger for all analog data
  DynamicUniversalDataLogger< cm_main > logger_;

  double V_th_;
};


inline port
nest::cm_main::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
cm_main::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( ( receptor_type < 0 ) or ( receptor_type >= static_cast< port >( syn_receptors_.size() ) ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }
  return receptor_type;
}

inline port
cm_main::handles_test_event( CurrentEvent&, rport receptor_type )
{
  // if get_compartment returns nullptr, raise the error
  if ( !c_tree_.get_compartment( long(receptor_type), c_tree_.get_root(), 0 ) )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return receptor_type;
}

inline port
cm_main::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
cm_main::get_status( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
cm_main::set_status( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_th, V_th_ );
  Archiving_Node::set_status( d );
}

} // namespace

#endif /* #ifndef IAF_NEAT_H */
