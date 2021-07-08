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
#include "cm_compartmentcurrents.h"

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

.. code-block:: Python
    cm =  nest.Create('cm_main')

users add compartments using the `nest.add_compartment()` function

.. code-block:: Python
    comp = nest.AddCompartment(cm, [compartment index], [parent index],
                                   [dictionary with compartment params])

After all compartments have been added, users can add receptors

.. code-block:: Python
    recept = nest.AddReceptor(cm, [compartment index], ['AMPA', 'GABA' or 'AMPA+NMDA'])

Compartment voltages can be recorded. To do so, users create a multimeter in the
standard manner but specify the to be recorded voltages as
'v_comp{compartment_index}'. Ion channels state variables can be recorded as well,
using the syntax '{state_variable_name}{compartment_index}'. For receptor state
variables, use the receptor index '{state_variable_name}{receptor_index}' i.e.

.. code-block:: Python
    mm = nest.Create('multimeter', 1, {'record_from': ['v_comp{compartment_index]'}, ...})

Current generators can be connected to the model. In this case, the receptor
type is the [compartment index], i.e.

.. code-block:: Python
    dc = nest.Create('dc_generator', {...})
    nest.Connect(dc, cm, syn_spec={..., 'receptor_type': [compartment index]}

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

Ion channels and receptor types for the default model are hardcoded.
For ion channels, there is a Na-channel and a K-channel. Parameters can be set
by specifying the following entries in the `AddCompartment` dictionary argument:

=========== ======= ===========================================================
 gbar_Na    uS      Maximal conductance Na channel
 e_Na       mV      Reversal Na channel
 gbar_K     uS      Maximal conductance K channel
 e_K        mV      Reversal K channel
=========== ======= ===========================================================

For receptors, the choice is from 'AMPA', 'GABA' or 'NMDA' or 'AMPA_NMDA'.
Ion channels and receptor types can be customized with NESTML.

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

References
++++++++++

Data-driven reduction of dendritic morphologies with preserved dendro-somatic responses
WAM Wybo, J Jordan, B Ellenberger, UM Mengual, T Nevian, W Senn
Elife 10, `e60936 <https://elifesciences.org/articles/60936>`_

See also
++++++++

NEURON simulator ;-D

EndUserDocs*/

class cm_main : public ArchivingNode
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

  /*
  Function to a compartment to the tree, so that the new compartment has the
  compartment specified by ``parent_compartment_idx`` as parent. The parent
  has to be in the tree, otherwise an error will be raised.
  */
  void add_compartment( const long compartment_idx, const long parent_compartment_idx, const DictionaryDatum& compartment_params ) override;
  /*
  Function to a add a receptor to a compartment. Returns the index of the
  receptor in the receptor stack.
  */
  size_t add_receptor( const long compartment_idx, const std::string& type, const DictionaryDatum& receptor_params ) override;

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void init_pointers_();

  void calibrate();

  void update( Time const&, const long, const long );

  CompTree c_tree_;
  std::vector< std::shared_ptr< RingBuffer > > syn_buffers_;

  // To record variables with DataAccessFunctor
  double get_state_element( size_t elem ){ return *recordables_values[elem]; };

  // The next classes need to be friends to access the State_ class/member
  friend class DataAccessFunctor< cm_main >;
  friend class DynamicRecordablesMap< cm_main >;
  friend class DynamicUniversalDataLogger< cm_main >;

  /*
  internal ordering of all recordables in a vector
  the vector 'recordables_values' stores pointers to all state variables
  present in the model
  */
  std::vector< std::string > recordables_names;
  std::vector< double* > recordables_values;

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
  if ( ( receptor_type < 0 ) or ( receptor_type >= static_cast< port >( syn_buffers_.size() ) ) )
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
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
cm_main::set_status( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_th, V_th_ );
  ArchivingNode::set_status( d );
}

} // namespace

#endif /* #ifndef IAF_NEAT_H */
