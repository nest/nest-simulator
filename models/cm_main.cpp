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
  , recordables_counter( 0 )
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
  , recordables_counter( 0 )
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
cm_main::add_compartment( const long compartment_idx, const long parent_compartment_idx, const DictionaryDatum& compartment_params )
{

  int kk = 0;
  std::cout << std::endl;
  std::cout << "add_compartment 0" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }


  if (compartment_idx > 1){
    std::cout << "add_compartment 0.5" << std::endl;

    Compartment* ccc = c_tree_.get_compartment( 0 );
    std::map< std::string, double* > rrr = ccc->get_recordables();

    for (auto rec_it = rrr.begin(); rec_it != rrr.end(); rec_it++)
    {
      std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;
    }
    Compartment* cccc = c_tree_.get_compartment( 1 );
    std::map< std::string, double* > rrrr = cccc->get_recordables();

    for (auto rec_it = rrrr.begin(); rec_it != rrrr.end(); rec_it++)
    {
      std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;
    }
  }

  c_tree_.add_compartment( compartment_idx, parent_compartment_idx, compartment_params);


  if (compartment_idx > 1){
    std::cout << "add_compartment 1.5" << std::endl;

    Compartment* ccc = c_tree_.get_compartment( 0 );
    std::map< std::string, double* > rrr = ccc->get_recordables();

    for (auto rec_it = rrr.begin(); rec_it != rrr.end(); rec_it++)
    {
      std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;
    }
    Compartment* cccc = c_tree_.get_compartment( 1 );
    std::map< std::string, double* > rrrr = cccc->get_recordables();

    for (auto rec_it = rrrr.begin(); rec_it != rrrr.end(); rec_it++)
    {
      std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;
    }
  }


  kk = 0;
  std::cout << "add_compartment 1" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }

  // get the map of all recordables (i.e. all state variables) of the compartment
  Compartment* compartment = c_tree_.get_compartment( compartment_idx );
  std::map< std::string, double* > recordables = compartment->get_recordables();


  kk = 0;
  std::cout << "add_compartment 2" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }

  if (compartment_idx > 1){
    std::cout << "add_compartment 2.5" << std::endl;

    Compartment* ccc = c_tree_.get_compartment( 0 );
    std::map< std::string, double* > rrr = ccc->get_recordables();

    for (auto rec_it = rrr.begin(); rec_it != rrr.end(); rec_it++)
    {
      std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;
    }
    Compartment* cccc = c_tree_.get_compartment( 1 );
    std::map< std::string, double* > rrrr = cccc->get_recordables();

    for (auto rec_it = rrrr.begin(); rec_it != rrrr.end(); rec_it++)
    {
      std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;
    }
  }

  for (auto rec_it = recordables.begin(); rec_it != recordables.end(); rec_it++)
  {


    // std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;

    // add the recordable value (i.e. pointer to the state variable) to the vector
    recordables_names.push_back( rec_it->first );
    recordables_values.push_back( rec_it->second );
    // add the recordable to the recordable_name -> recordable_index map
    recordablesMap_.insert( rec_it->first + std::to_string(compartment_idx),
                            DataAccessFunctor< cm_main >( *this, recordables_counter ) );

    recordables_counter++;
  }


  kk = 0;
  std::cout << "add_compartment 3" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }

}

size_t
cm_main::add_receptor( const long compartment_idx, const std::string& type, const DictionaryDatum& receptor_params )
{


  int kk = 0;
  std::cout << std::endl;
  std::cout << "add_receptor 1" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }

  // create a ringbuffer to collect spikes for the receptor
  std::shared_ptr< RingBuffer > buffer = std::shared_ptr< RingBuffer >( new RingBuffer() );

  // add the ringbuffer to the global receptor vector
  const size_t syn_idx = syn_buffers_.size();
  syn_buffers_.push_back( buffer );

  // add the receptor to the compartment
  Compartment* compartment = c_tree_.get_compartment( compartment_idx );
  std::map< std::string, double* > recordables =
          compartment->compartment_currents.add_synapse_with_buffer( type, buffer, receptor_params );

  // add the recordables for the receptor
  for (auto rec_it = recordables.begin(); rec_it != recordables.end(); rec_it++)
  {
    std::cout << "recordable " << rec_it->first << " = " << rec_it->second << "->" << *rec_it->second << std::endl;

    // add the recordable value (i.e. pointer to the state variable) to the vector
    recordables_names.push_back( rec_it->first );
    recordables_values.push_back( rec_it->second );
    // add the recordable to the recordable_name -> recordable_index map
    recordablesMap_.insert( rec_it->first + std::to_string(syn_idx),
                            DataAccessFunctor< cm_main >( *this, recordables_counter ) );

    recordables_counter++;
  }


  kk = 0;
  std::cout << "add_receptor 2" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }

  return syn_idx;
}

void
nest::cm_main::calibrate()
{

  int kk = 0;
  std::cout << "calibrate 1" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }

  logger_.init();
  c_tree_.init();

  kk = 0;
  std::cout << "calibrate 2" << std::endl;
  for (auto rec_it = recordables_values.begin(); rec_it != recordables_values.end(); rec_it++)
  {
    std::cout << "recordables[" << kk << "] = " << recordables_names[kk] << ": " << *rec_it << "->" << **rec_it << std::endl;
    kk++;
  }
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
      // c_tree_.get_root()->etype.add_spike();

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

  syn_buffers_[ e.get_rport() ]->add_value(
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
