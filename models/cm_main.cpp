/*
 *  cm_main.cpp
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
#include "cm_main.h"


namespace nest
{


template <>
void
DynamicRecordablesMap< cm_main >::create( cm_main& host)
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::cm_main::cm_main()
  : ArchivingNode()
  , c_tree_()
  , syn_buffers_( 0 )
  , logger_( *this )
  , V_th_( -55.0 )
{
  recordablesMap_.create( *this );
  recordables_values.resize( 0 );
}

nest::cm_main::cm_main( const cm_main& n )
  : ArchivingNode( n )
  , c_tree_( n.c_tree_ )
  , syn_buffers_( n.syn_buffers_ )
  , logger_( *this )
  , V_th_( n.V_th_ )
{
  recordables_values.resize( 0 );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::cm_main::init_state_( const Node& proto )
{
}

void
nest::cm_main::init_buffers_()
{
  logger_.reset();
  ArchivingNode::clear_history();
}

void
nest::cm_main::add_compartment( const long compartment_idx, const long parent_compartment_idx, const DictionaryDatum& compartment_params )
{
  c_tree_.add_compartment( compartment_idx, parent_compartment_idx, compartment_params);

  // we need to initialize tree pointers because vectors are resized, thus
  // moving memory addresses
  init_tree_pointers_();
  // we need to initialize the recordables pointers to guarantee that the
  // recordables of the new compartment will be in the recordables map
  init_recordables_pointers_();
}

size_t
nest::cm_main::add_receptor( const long compartment_idx, const std::string& type, const DictionaryDatum& receptor_params )
{
  // create a ringbuffer to collect spikes for the receptor
  RingBuffer buffer;

  // add the ringbuffer to the global receptor vector
  const size_t syn_idx = syn_buffers_.size();
  syn_buffers_.push_back( buffer );

  // add the receptor to the compartment
  Compartment* compartment = c_tree_.get_compartment( compartment_idx );
  compartment->compartment_currents.add_synapse( type, syn_idx, receptor_params );

  // we need to initialize the recordables pointers to guarantee that the
  // recordables of the new synapse will be in the recordables map
  init_recordables_pointers_();

  return syn_idx;
}

/*
The following functions initialize the internal pointers of the compartmental
model.
*/
void
nest::cm_main::init_tree_pointers_()
{
  /*
  initialize the pointers within the compartment tree
  */
  c_tree_.init_pointers();
}
void
nest::cm_main::init_syn_pointers_()
{
  /*
  initialize the pointers to the synapse buffers for the receptor currents
  */
  c_tree_.set_syn_buffers( syn_buffers_ );
}
void
nest::cm_main::init_recordables_pointers_()
{
  /*
  Get the map of all recordables (i.e. all state variables of the model):
  --> keys are state variable names suffixed by the compartment index for
      voltage (e.g. "v_comp1") or by the synapse index for receptor currents
  --> values are pointers to the specific state variables
  */
  std::map< std::string, double* > recordables = c_tree_.get_recordables();

  for (auto rec_it = recordables.begin(); rec_it != recordables.end(); rec_it++)
  {
    // check if name is already in recordables map
    auto recname_it = find(recordables_names.begin(), recordables_names.end(), rec_it->first);
    if( recname_it == recordables_names.end() )
    {
      // recordable name is not yet in map, we need to add it
      recordables_names.push_back( rec_it->first );
      recordables_values.push_back( rec_it->second );
      const long rec_idx = recordables_values.size() - 1;
      // add the recordable to the recordable_name -> recordable_index map
      recordablesMap_.insert( rec_it->first,
                              DataAccessFunctor< cm_main >( *this, rec_idx ) );
    }
    else
    {
      // recordable name is in map, we update the pointer to the recordable
      long index = recname_it - recordables_names.begin();
      recordables_values[index] = rec_it->second;
    }
  }
}


void
nest::cm_main::calibrate()
{
  logger_.init();

  init_tree_pointers_();
  init_syn_pointers_();
  init_recordables_pointers_();
  c_tree_.init();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */
void
nest::cm_main::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    const double v_0_prev = c_tree_.get_root()->v_comp;

    c_tree_.construct_matrix( lag );
    c_tree_.solve_matrix();

    // threshold crossing
    if ( c_tree_.get_root()->v_comp >= V_th_ && v_0_prev < V_th_ )
    {
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::cm_main::handle( SpikeEvent& e )
{
  if ( e.get_weight() < 0 )
  {
    throw BadProperty(
      "Synaptic weights must be positive." );
  }

  assert( e.get_delay_steps() > 0 );
  assert( ( e.get_rport() >= 0 ) && ( ( size_t ) e.get_rport() < syn_buffers_.size() ) );

  syn_buffers_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps(kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::cm_main::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  Compartment* compartment = c_tree_.get_compartment( e.get_rport() );
  compartment->currents.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::cm_main::handle( DataLoggingRequest& e )
{
  logger_.handle( e );
}

} // namespace
