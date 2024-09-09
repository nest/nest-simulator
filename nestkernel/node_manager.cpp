/*
 *  node_manager.cpp
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

#include "node_manager.h"

// C++ includes:
#include <algorithm>
#include <set>

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"
#include "node.h"
#include "secondary_event_impl.h"
#include "vp_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

NodeManager::NodeManager()
  : local_nodes_( 1 )
  , node_collection_container_()
  , wfr_nodes_vec_()
  , wfr_is_used_( false )
  , wfr_network_size_( 0 ) // zero to force update
  , num_active_nodes_( 0 )
  , num_thread_local_devices_()
  , have_nodes_changed_( true )
  , exceptions_raised_() // cannot call kernel(), not complete yet
{
}

NodeManager::~NodeManager()
{
  // We must destruct nodes here, since devices may need to close files.
  destruct_nodes_();
  clear_node_collection_container();
}

void
NodeManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  // explicitly force construction of wfr_nodes_vec_ to ensure consistent state
  wfr_network_size_ = 0;
  local_nodes_.resize( kernel().vp_manager.get_num_threads() );
  num_thread_local_devices_.resize( kernel().vp_manager.get_num_threads(), 0 );
  ensure_valid_thread_local_ids();

  if ( not adjust_number_of_threads_or_rng_only )
  {
    sw_construction_create_.reset();
  }
}

void
NodeManager::finalize( const bool )
{
  destruct_nodes_();
  clear_node_collection_container();
}

DictionaryDatum
NodeManager::get_status( size_t idx )
{
  Node* target = get_mpi_local_node_or_device_head( idx );

  assert( target );

  DictionaryDatum d = target->get_status_base();

  return d;
}

NodeCollectionPTR
NodeManager::add_node( size_t model_id, long n )
{
  sw_construction_create_.start();

  have_nodes_changed_ = true;

  if ( n < 1 )
  {
    throw BadProperty();
  }

  Model* model = kernel().model_manager.get_node_model( model_id );
  assert( model );
  model->deprecation_warning( "Create" );

  const size_t min_node_id = local_nodes_.at( 0 ).get_max_node_id() + 1;
  const size_t max_node_id = min_node_id + n - 1;
  if ( max_node_id < min_node_id )
  {
    LOG( M_ERROR,
      "NodeManager::add_node",
      "Requested number of nodes will overflow the memory. "
      "No nodes were created" );
    throw KernelException( "OutOfMemory" );
  }

  kernel().modelrange_manager.add_range( model_id, min_node_id, max_node_id );

  // clear any exceptions from previous call
  std::vector< std::shared_ptr< WrappedThreadException > >( kernel().vp_manager.get_num_threads() )
    .swap( exceptions_raised_ );

  auto nc_ptr = NodeCollectionPTR( new NodeCollectionPrimitive( min_node_id, max_node_id, model_id ) );
  append_node_collection_( nc_ptr );

  if ( model->has_proxies() )
  {
    add_neurons_( *model, min_node_id, max_node_id );
  }
  else if ( not model->one_node_per_process() )
  {
    add_devices_( *model, min_node_id, max_node_id );
  }
  else
  {
    add_music_nodes_( *model, min_node_id, max_node_id );
  }

  // check if any exceptions have been raised
  for ( size_t t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    if ( exceptions_raised_.at( t ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( t ) ) );
    }
  }

  // activate off-grid communication only after nodes have been created
  // successfully
  if ( model->is_off_grid() )
  {
    kernel().event_delivery_manager.set_off_grid_communication( true );
    LOG( M_INFO,
      "NodeManager::add_node",
      "Neuron models emitting precisely timed spikes exist: "
      "the kernel property off_grid_spiking has been set to true.\n\n"
      "NOTE: Mixing precise-spiking and normal neuron models may "
      "lead to inconsistent results." );
  }

  // resize the target table for delivery of events to devices to make sure the first dimension
  // matches the number of local nodes and the second dimension matches number of synapse types
  kernel().connection_manager.resize_target_table_devices_to_number_of_neurons();

#pragma omp parallel
  {
    // must be called in parallel context to properly configure per-thread data structures
    kernel().connection_manager.resize_target_table_devices_to_number_of_synapse_types();
  }

  sw_construction_create_.stop();

  return nc_ptr;
}

void
NodeManager::add_neurons_( Model& model, size_t min_node_id, size_t max_node_id )
{
  const size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  // Upper limit for number of neurons per thread; in practice, either
  // max_new_per_thread-1 or max_new_per_thread nodes will be created.
  const size_t max_new_per_thread =
    static_cast< size_t >( std::ceil( static_cast< double >( max_node_id - min_node_id + 1 ) / num_vps ) );

#pragma omp parallel
  {
    const size_t t = kernel().vp_manager.get_thread_id();

    try
    {
      model.reserve_additional( t, max_new_per_thread );
      // Need to find smallest node ID with:
      //   - node ID local to this vp
      //   - node_id >= min_node_id
      const size_t vp = kernel().vp_manager.thread_to_vp( t );
      const size_t min_node_id_vp = kernel().vp_manager.node_id_to_vp( min_node_id );

      size_t node_id = min_node_id + ( num_vps + vp - min_node_id_vp ) % num_vps;

      while ( node_id <= max_node_id )
      {
        Node* node = model.create( t );
        node->set_node_id_( node_id );
        node->set_model_id( model.get_model_id() );
        node->set_thread( t );
        node->set_vp( vp );
        node->set_initialized();

        local_nodes_[ t ].add_local_node( *node );
        node_id += num_vps;
      }
      local_nodes_[ t ].set_max_node_id( max_node_id );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  } // omp parallel
}

void
NodeManager::add_devices_( Model& model, size_t min_node_id, size_t max_node_id )
{
  const size_t n_per_thread = max_node_id - min_node_id + 1;

#pragma omp parallel
  {
    const size_t t = kernel().vp_manager.get_thread_id();
    try
    {
      model.reserve_additional( t, n_per_thread );

      for ( size_t node_id = min_node_id; node_id <= max_node_id; ++node_id )
      {
        // keep track of number of thread local devices
        ++num_thread_local_devices_[ t ];

        Node* node = model.create( t );
        node->set_node_id_( node_id );
        node->set_model_id( model.get_model_id() );
        node->set_thread( t );
        node->set_vp( kernel().vp_manager.thread_to_vp( t ) );
        node->set_local_device_id( num_thread_local_devices_[ t ] - 1 );
        node->set_initialized();

        local_nodes_[ t ].add_local_node( *node );
      }
      local_nodes_[ t ].set_max_node_id( max_node_id );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  } // omp parallel
}

void
NodeManager::add_music_nodes_( Model& model, size_t min_node_id, size_t max_node_id )
{
#pragma omp parallel
  {
    const size_t t = kernel().vp_manager.get_thread_id();
    try
    {
      if ( t == 0 )
      {
        for ( size_t node_id = min_node_id; node_id <= max_node_id; ++node_id )
        {
          // keep track of number of thread local devices
          ++num_thread_local_devices_[ t ];

          Node* node = model.create( 0 );
          node->set_node_id_( node_id );
          node->set_model_id( model.get_model_id() );
          node->set_thread( 0 );
          node->set_vp( kernel().vp_manager.thread_to_vp( 0 ) );
          node->set_local_device_id( num_thread_local_devices_[ t ] - 1 );
          node->set_initialized();

          local_nodes_[ 0 ].add_local_node( *node );
        }
      }
      local_nodes_.at( t ).set_max_node_id( max_node_id );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  } // omp parallel
}

NodeCollectionPTR
NodeManager::node_id_to_node_collection( const size_t node_id ) const
{
  // find the largest ID in node_collection_last_ that is still smaller than node_id
  auto it = std::lower_bound( node_collection_last_.begin(), node_collection_last_.end(), node_id );

  // compute the position of the nodeCollection based on the position of the ID found above
  size_t pos = it - node_collection_last_.begin();
  return node_collection_container_.at( pos );
}

NodeCollectionPTR
NodeManager::node_id_to_node_collection( Node* node ) const
{
  return node_id_to_node_collection( node->get_node_id() );
}

void
NodeManager::append_node_collection_( NodeCollectionPTR ncp )
{
  node_collection_container_.push_back( ncp );
  node_collection_last_.push_back( ncp->get_last() );
}

void
NodeManager::clear_node_collection_container()
{
  node_collection_container_.clear();
  node_collection_last_.clear();
}

NodeCollectionPTR
NodeManager::get_nodes( const DictionaryDatum& params, const bool local_only )
{
  std::vector< long > nodes;

  if ( params->empty() )
  {
    std::vector< std::vector< long > > nodes_on_thread;
    nodes_on_thread.resize( kernel().vp_manager.get_num_threads() );
#pragma omp parallel
    {
      size_t tid = kernel().vp_manager.get_thread_id();

      for ( auto node : get_local_nodes( tid ) )
      {
        nodes_on_thread[ tid ].push_back( node.get_node_id() );
      }
    } // omp parallel

#pragma omp barrier

    for ( auto vec : nodes_on_thread )
    {
      nodes.insert( nodes.end(), vec.begin(), vec.end() );
    }
  }
  else
  {
    for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
    {
      // Select those nodes fulfilling the key/value pairs of the dictionary
      for ( auto node : get_local_nodes( tid ) )
      {
        bool match = true;
        size_t node_id = node.get_node_id();

        DictionaryDatum node_status = get_status( node_id );
        for ( Dictionary::iterator dict_entry = params->begin(); dict_entry != params->end(); ++dict_entry )
        {
          if ( node_status->known( dict_entry->first ) )
          {
            const Token token = node_status->lookup( dict_entry->first );
            if ( not( token == dict_entry->second or token.matches_as_string( dict_entry->second ) ) )
            {
              match = false;
              break;
            }
          }
        }
        if ( match )
        {
          nodes.push_back( node_id );
        }
      }
    }
  }

  if ( not local_only )
  {
    std::vector< long > globalnodes;
    kernel().mpi_manager.communicate( nodes, globalnodes );

    for ( size_t i = 0; i < globalnodes.size(); ++i )
    {
      if ( globalnodes[ i ] )
      {
        nodes.push_back( globalnodes[ i ] );
      }
    }

    // get rid of any multiple entries
    std::sort( nodes.begin(), nodes.end() );
    std::vector< long >::iterator it;
    it = std::unique( nodes.begin(), nodes.end() );
    nodes.resize( it - nodes.begin() );
  }

  std::sort( nodes.begin(), nodes.end() ); // ensure nodes are sorted prior to creating the NodeCollection
  IntVectorDatum nodes_datum( nodes );
  NodeCollectionDatum nodecollection( NodeCollection::create( nodes_datum ) );

  return std::move( nodecollection );
}

bool
NodeManager::is_local_node( Node* n ) const
{
  return kernel().vp_manager.is_local_vp( n->get_vp() );
}

bool
NodeManager::is_local_node_id( size_t node_id ) const
{
  const size_t vp = kernel().vp_manager.node_id_to_vp( node_id );
  return kernel().vp_manager.is_local_vp( vp );
}

size_t
NodeManager::get_max_num_local_nodes() const
{
  return static_cast< size_t >(
    ceil( static_cast< double >( size() ) / kernel().vp_manager.get_num_virtual_processes() ) );
}

size_t
NodeManager::get_num_thread_local_devices( size_t t ) const
{
  return num_thread_local_devices_[ t ];
}

Node*
NodeManager::get_node_or_proxy( size_t node_id, size_t t )
{
  assert( t < kernel().vp_manager.get_num_threads() );
  assert( node_id <= size() );

  Node* node = local_nodes_[ t ].get_node_by_node_id( node_id );
  if ( not node )
  {
    return kernel().model_manager.get_proxy_node( t, node_id );
  }

  return node;
}

Node*
NodeManager::get_node_or_proxy( size_t node_id )
{
  assert( 0 < node_id and node_id <= size() );

  size_t vp = kernel().vp_manager.node_id_to_vp( node_id );
  if ( not kernel().vp_manager.is_local_vp( vp ) )
  {
    return kernel().model_manager.get_proxy_node( 0, node_id );
  }

  size_t t = kernel().vp_manager.vp_to_thread( vp );
  Node* node = local_nodes_[ t ].get_node_by_node_id( node_id );
  if ( not node )
  {
    return kernel().model_manager.get_proxy_node( t, node_id );
  }

  return node;
}

Node*
NodeManager::get_mpi_local_node_or_device_head( size_t node_id )
{
  size_t t = kernel().vp_manager.vp_to_thread( kernel().vp_manager.node_id_to_vp( node_id ) );

  Node* node = local_nodes_[ t ].get_node_by_node_id( node_id );

  if ( not node )
  {
    return kernel().model_manager.get_proxy_node( t, node_id );
  }
  if ( not node->has_proxies() )
  {
    node = local_nodes_[ 0 ].get_node_by_node_id( node_id );
  }

  return node;
}

std::vector< Node* >
NodeManager::get_thread_siblings( size_t node_id ) const
{
  size_t num_threads = kernel().vp_manager.get_num_threads();
  std::vector< Node* > siblings( num_threads );
  for ( size_t t = 0; t < num_threads; ++t )
  {
    Node* node = local_nodes_[ t ].get_node_by_node_id( node_id );
    if ( not node )
    {
      throw NoThreadSiblingsAvailable( node_id );
    }

    siblings[ t ] = node;
  }

  return siblings;
}

void
NodeManager::ensure_valid_thread_local_ids()
{
  // Check if the network size changed, in order to not enter
  // the critical region if it is not necessary. Note that this
  // test also covers that case that nodes have been deleted
  // by reset.
  if ( size() == wfr_network_size_ )
  {
    return;
  }

#pragma omp critical( update_wfr_nodes_vec )
  {
    // This code may be called from a thread-parallel context, when it is
    // invoked by TargetIdentifierIndex::set_target() during parallel
    // wiring. Nested OpenMP parallelism is problematic, therefore, we
    // enforce single threading here. This should be unproblematic wrt
    // performance, because the wfr_nodes_vec_ is rebuilt only once after
    // changes in network size.
    //
    // Check again, if the network size changed, since a previous thread
    // can have updated wfr_nodes_vec_ before.
    if ( size() != wfr_network_size_ )
    {

      // We clear the existing wfr_nodes_vec_ and then rebuild it.
      wfr_nodes_vec_.clear();
      wfr_nodes_vec_.resize( kernel().vp_manager.get_num_threads() );

      for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
      {
        wfr_nodes_vec_[ tid ].clear();

        const size_t num_thread_local_wfr_nodes = std::count_if( local_nodes_[ tid ].begin(),
          local_nodes_[ tid ].end(),
          []( const SparseNodeArray::NodeEntry& elem ) { return elem.get_node()->node_uses_wfr_; } );
        wfr_nodes_vec_[ tid ].reserve( num_thread_local_wfr_nodes );

        auto node_it = local_nodes_[ tid ].begin();
        size_t idx = 0;
        for ( ; node_it < local_nodes_[ tid ].end(); ++node_it, ++idx )
        {
          auto node = node_it->get_node();
          node->set_thread_lid( idx );
          if ( node->node_uses_wfr_ )
          {
            wfr_nodes_vec_[ tid ].push_back( node );
          }
        }
      } // end of for threads

      wfr_network_size_ = size();

      // wfr_is_used_ indicates, whether at least one
      // of the threads has a neuron that uses waveform relaxation
      // all threads then need to perform a wfr_update
      // step, because gather_events() has to be done in an
      // openmp single section
      wfr_is_used_ = false;
      for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
      {
        if ( wfr_nodes_vec_[ tid ].size() > 0 )
        {
          wfr_is_used_ = true;
        }
      }
    }
  } // omp critical
}

void
NodeManager::destruct_nodes_()
{
#pragma omp parallel
  {
    const size_t tid = kernel().vp_manager.get_thread_id();
    for ( auto node : local_nodes_[ tid ] )
    {
      delete node.get_node();
    }
    local_nodes_[ tid ].clear();
  } // omp parallel
}

void
NodeManager::set_status_single_node_( Node& target, const DictionaryDatum& d, bool clear_flags )
{
  // proxies have no properties
  if ( not target.is_proxy() )
  {
    if ( clear_flags )
    {
      d->clear_access_flags();
    }
    target.set_status_base( d );

    // TODO: Not sure this check should be at single neuron level; advantage is
    // it stops after first failure.
    ALL_ENTRIES_ACCESSED( *d, "NodeManager::set_status", "Unread dictionary entries: " );
  }
}

void
NodeManager::prepare_node_( Node* n )
{
  // Frozen nodes are initialized and calibrated, so that they
  // have ring buffers and can accept incoming spikes.
  n->init();
  n->pre_run_hook();
}

void
NodeManager::prepare_nodes()
{
  assert( kernel().is_initialized() );

  // We initialize the buffers of each node and calibrate it.

  size_t num_active_nodes = 0;     // counts nodes that will be updated
  size_t num_active_wfr_nodes = 0; // counts nodes that use waveform relaxation

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised( kernel().vp_manager.get_num_threads() );

#pragma omp parallel reduction( + : num_active_nodes, num_active_wfr_nodes )
  {
    size_t t = kernel().vp_manager.get_thread_id();

    // We prepare nodes in a parallel region. Therefore, we need to catch
    // exceptions here and then handle them after the parallel region.
    try
    {
      for ( SparseNodeArray::const_iterator it = local_nodes_[ t ].begin(); it != local_nodes_[ t ].end(); ++it )
      {
        prepare_node_( ( it )->get_node() );
        if ( not( it->get_node() )->is_frozen() )
        {
          ++num_active_nodes;
          if ( ( it->get_node() )->node_uses_wfr() )
          {
            ++num_active_wfr_nodes;
          }
        }
      }
    }
    catch ( std::exception& e )
    {
      // so throw the exception after parallel region
      exceptions_raised.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( e ) );
    }
  } // omp parallel

  // check if any exceptions have been raised
  for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised.at( tid ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised.at( tid ) ) );
    }
  }

  std::ostringstream os;
  std::string tmp_str = num_active_nodes == 1 ? " node" : " nodes";
  os << "Preparing " << num_active_nodes << tmp_str << " for simulation.";

  if ( num_active_wfr_nodes != 0 )
  {
    tmp_str = num_active_wfr_nodes == 1 ? " uses " : " use ";
    os << " " << num_active_wfr_nodes << " of them" << tmp_str << "iterative solution techniques.";
  }

  num_active_nodes_ = num_active_nodes;
  LOG( M_INFO, "NodeManager::prepare_nodes", os.str() );
}

void
NodeManager::post_run_cleanup()
{
#pragma omp parallel
  {
    size_t t = kernel().vp_manager.get_thread_id();
    SparseNodeArray::const_iterator n;
    for ( n = local_nodes_[ t ].begin(); n != local_nodes_[ t ].end(); ++n )
    {
      n->get_node()->post_run_cleanup();
    }
  } // omp parallel
}

void
NodeManager::finalize_nodes()
{
#pragma omp parallel
  {
    size_t tid = kernel().vp_manager.get_thread_id();
    SparseNodeArray::const_iterator n;
    for ( n = local_nodes_[ tid ].begin(); n != local_nodes_[ tid ].end(); ++n )
    {
      n->get_node()->finalize();
    }
  } // omp parallel
}

void
NodeManager::check_wfr_use()
{
  wfr_is_used_ = kernel().mpi_manager.any_true( wfr_is_used_ );

  GapJunctionEvent::set_coeff_length(
    kernel().connection_manager.get_min_delay() * ( kernel().simulation_manager.get_wfr_interpolation_order() + 1 ) );
  InstantaneousRateConnectionEvent::set_coeff_length( kernel().connection_manager.get_min_delay() );
  DelayedRateConnectionEvent::set_coeff_length( kernel().connection_manager.get_min_delay() );
  DiffusionConnectionEvent::set_coeff_length( kernel().connection_manager.get_min_delay() );
  LearningSignalConnectionEvent::set_coeff_length( kernel().connection_manager.get_min_delay() );
  SICEvent::set_coeff_length( kernel().connection_manager.get_min_delay() );
}

void
NodeManager::print( std::ostream& out ) const
{
  const size_t max_node_id = size();
  const double max_node_id_width = std::floor( std::log10( max_node_id ) );
  const double node_id_range_width = 6 + 2 * max_node_id_width;

  for ( std::vector< modelrange >::const_iterator it = kernel().modelrange_manager.begin();
        it != kernel().modelrange_manager.end();
        ++it )
  {
    const size_t first_node_id = it->get_first_node_id();
    const size_t last_node_id = it->get_last_node_id();
    const Model* mod = kernel().model_manager.get_node_model( it->get_model_id() );

    std::stringstream node_id_range_strs;
    node_id_range_strs << std::setw( max_node_id_width + 1 ) << first_node_id;
    if ( last_node_id != first_node_id )
    {
      node_id_range_strs << " .. " << std::setw( max_node_id_width + 1 ) << last_node_id;
    }
    out << std::setw( node_id_range_width ) << std::left << node_id_range_strs.str() << " " << mod->get_name();

    if ( it + 1 != kernel().modelrange_manager.end() )
    {
      out << std::endl;
    }
  }
}

void
NodeManager::set_status( size_t node_id, const DictionaryDatum& d )
{
  for ( size_t t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    Node* node = local_nodes_[ t ].get_node_by_node_id( node_id );
    if ( node )
    {
      set_status_single_node_( *node, d );
    }
  }
}

void
NodeManager::get_status( DictionaryDatum& d )
{
  def< long >( d, names::network_size, size() );
  def< double >( d, names::time_construction_create, sw_construction_create_.elapsed() );
}

void
NodeManager::set_status( const DictionaryDatum& )
{
}

} // namespace nest
