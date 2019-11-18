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
#include <set>

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "event_delivery_manager.h"
#include "genericmodel.h"
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"
#include "node.h"
#include "vp_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

NodeManager::NodeManager()
  : local_nodes_( 1 )
  , wfr_nodes_vec_()
  , wfr_is_used_( false )
  , wfr_network_size_( 0 ) // zero to force update
  , num_active_nodes_( 0 )
  , num_local_devices_( 0 )
  , have_nodes_changed_( true )
  , exceptions_raised_() // cannot call kernel(), not complete yet
{
}

NodeManager::~NodeManager()
{
  destruct_nodes_(); // We must destruct nodes properly, since devices may need
                     // to close files.
}

void
NodeManager::initialize()
{
  // explicitly force construction of wfr_nodes_vec_ to ensure consistent state
  wfr_network_size_ = 0;
  local_nodes_.resize( kernel().vp_manager.get_num_threads() );
  ensure_valid_thread_local_ids();

  num_local_devices_ = 0;
}

void
NodeManager::finalize()
{
  destruct_nodes_();
}

DictionaryDatum
NodeManager::get_status( index idx )
{
  Node* target = get_mpi_local_node_or_device_head( idx );

  assert( target != 0 );

  DictionaryDatum d = target->get_status_base();

  return d;
}

GIDCollectionPTR
NodeManager::add_node( index model_id, long n )
{
  have_nodes_changed_ = true;

  if ( model_id >= kernel().model_manager.get_num_node_models() )
  {
    throw UnknownModelID( model_id );
  }

  if ( n < 1 )
  {
    throw BadProperty();
  }

  Model* model = kernel().model_manager.get_model( model_id );
  assert( model != 0 );
  model->deprecation_warning( "Create" );

  const index min_gid = local_nodes_.at( 0 ).get_max_gid() + 1;
  const index max_gid = min_gid + n - 1;
  if ( max_gid < min_gid )
  {
    LOG( M_ERROR,
      "NodeManager::add_node",
      "Requested number of nodes will overflow the memory. "
      "No nodes were created" );
    throw KernelException( "OutOfMemory" );
  }

  kernel().modelrange_manager.add_range( model_id, min_gid, max_gid );

  // clear any exceptions from previous call
  std::vector< std::shared_ptr< WrappedThreadException > >( kernel().vp_manager.get_num_threads() )
    .swap( exceptions_raised_ );

  auto gc_ptr = GIDCollectionPTR( new GIDCollectionPrimitive( min_gid, max_gid, model_id ) );

  if ( model->has_proxies() )
  {
    add_neurons_( *model, min_gid, max_gid, gc_ptr );
  }
  else if ( not model->one_node_per_process() )
  {
    add_devices_( *model, min_gid, max_gid, gc_ptr );
  }
  else
  {
    add_music_nodes_( *model, min_gid, max_gid, gc_ptr );
  }

  // check if any exceptions have been raised
  for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
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

  // resize the target table for delivery of events to devices to make
  // sure the first dimension matches the number of local nodes and
  // the second dimension matches number of synapse types
  kernel().connection_manager.resize_target_table_devices_to_number_of_neurons();
  kernel().connection_manager.resize_target_table_devices_to_number_of_synapse_types();

  return gc_ptr;
}


void
NodeManager::add_neurons_( Model& model, index min_gid, index max_gid, GIDCollectionPTR gc_ptr )
{
  // upper limit for number of neurons per thread; in practice, either
  // max_new_per_thread-1 or max_new_per_thread nodes will be created
  const size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  const size_t max_new_per_thread =
    static_cast< size_t >( std::ceil( static_cast< double >( max_gid - min_gid + 1 ) / num_vps ) );

#pragma omp parallel
  {
    const index t = kernel().vp_manager.get_thread_id();

    try
    {
      model.reserve_additional( t, max_new_per_thread );

      // Need to find smallest gid with:
      //   - gid local to this vp
      //   - gid >= min_gid
      const size_t vp = kernel().vp_manager.thread_to_vp( t );
      const size_t min_gid_vp = kernel().vp_manager.suggest_vp_for_gid( min_gid );

      size_t gid = min_gid + ( num_vps + vp - min_gid_vp ) % num_vps;

      while ( gid <= max_gid )
      {
        Node* node = model.allocate( t );
        node->set_gid_( gid );
        node->set_gc_( gc_ptr );
        node->set_model_id( model.get_model_id() );
        node->set_thread( t );
        node->set_vp( vp );
        node->set_initialized();

        local_nodes_[ t ].add_local_node( *node );
        gid += num_vps;
      }
      local_nodes_[ t ].update_max_gid( max_gid );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

void
NodeManager::add_devices_( Model& model, index min_gid, index max_gid, GIDCollectionPTR gc_ptr )
{
  const size_t n_per_thread = max_gid - min_gid + 1;

#pragma omp parallel
  {
    const index t = kernel().vp_manager.get_thread_id();
    try
    {
      model.reserve_additional( t, n_per_thread );

      for ( index gid = min_gid; gid <= max_gid; ++gid )
      {
        // keep track of number of local devices
        ++num_local_devices_;

        Node* node = model.allocate( t );
        node->set_gid_( gid );
        node->set_gc_( gc_ptr );
        node->set_model_id( model.get_model_id() );
        node->set_thread( t );
        node->set_vp( kernel().vp_manager.thread_to_vp( t ) );
        node->set_local_device_id( num_local_devices_ - 1 );
        node->set_initialized();

        local_nodes_[ t ].add_local_node( *node );
      }
      local_nodes_[ t ].update_max_gid( max_gid );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

void
NodeManager::add_music_nodes_( Model& model, index min_gid, index max_gid, GIDCollectionPTR gc_ptr )
{
#pragma omp parallel
  {
    const size_t t = kernel().vp_manager.get_thread_id();
    try
    {
      if ( t == 0 )
      {
        for ( index gid = min_gid; gid <= max_gid; ++gid )
        {
          // keep track of number of local devices
          ++num_local_devices_;

          Node* node = model.allocate( 0 );
          node->set_gid_( gid );
          node->set_gc_( gc_ptr );
          node->set_model_id( model.get_model_id() );
          node->set_thread( 0 );
          node->set_vp( kernel().vp_manager.thread_to_vp( 0 ) );
          node->set_local_device_id( num_local_devices_ - 1 );
          node->set_initialized();

          local_nodes_[ 0 ].add_local_node( *node );
        }
      }
      local_nodes_.at( t ).update_max_gid( max_gid );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( t ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

GIDCollectionPTR
NodeManager::get_nodes( const DictionaryDatum& params, const bool local_only )
{
  std::vector< long > nodes;

  if ( params->empty() )
  {
    std::vector< std::vector< long > > nodes_on_thread;
    nodes_on_thread.resize( kernel().vp_manager.get_num_threads() );
#pragma omp parallel
    {
      thread tid = kernel().vp_manager.get_thread_id();

      for ( auto node : get_local_nodes( tid ) )
      {
        nodes_on_thread[ tid ].push_back( node.get_gid() );
      }
    }
#pragma omp barrier

    for ( auto vec : nodes_on_thread )
    {
      nodes.insert( nodes.end(), vec.begin(), vec.end() );
    }
  }
  else
  {
    for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
    {
      // Select those nodes fulfilling the key/value pairs of the dictionary
      for ( auto node : get_local_nodes( tid ) )
      {
        bool match = true;
        index gid = node.get_gid();

        DictionaryDatum node_status = get_status( gid );
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
          nodes.push_back( gid );
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

  IntVectorDatum nodes_datum( nodes );
  GIDCollectionDatum gidcoll( GIDCollection::create( nodes_datum ) );

  return gidcoll;
}

void
NodeManager::init_state( index GID )
{
  Node* n = get_node_or_proxy( GID );
  if ( n == 0 )
  {
    throw UnknownNode( GID );
  }

  n->init_state();
}

bool
NodeManager::is_local_node( Node* n ) const
{
  return kernel().vp_manager.is_local_vp( n->get_vp() );
}

bool
NodeManager::is_local_gid( index gid ) const
{
  const thread vp = kernel().vp_manager.suggest_vp_for_gid( gid );
  return kernel().vp_manager.is_local_vp( vp );
}

index
NodeManager::get_max_num_local_nodes() const
{
  return static_cast< index >(
    ceil( static_cast< double >( size() ) / kernel().vp_manager.get_num_virtual_processes() ) );
}

index
NodeManager::get_num_local_devices() const
{
  return num_local_devices_;
}

Node*
NodeManager::get_node_or_proxy( index gid, thread t )
{
  assert( 0 <= t and ( t == -1 or t < kernel().vp_manager.get_num_threads() ) );
  assert( 0 < gid and gid <= size() );

  Node* node = local_nodes_[ t ].get_node_by_gid( gid );
  if ( node == 0 )
  {
    return kernel().model_manager.get_proxy_node( t, gid );
  }

  return node;
}

Node*
NodeManager::get_node_or_proxy( index gid )
{
  assert( 0 < gid and gid <= size() );

  thread vp = kernel().vp_manager.suggest_vp_for_gid( gid );
  if ( not kernel().vp_manager.is_local_vp( vp ) )
  {
    return kernel().model_manager.get_proxy_node( 0, gid );
  }

  thread t = kernel().vp_manager.vp_to_thread( vp );
  Node* node = local_nodes_[ t ].get_node_by_gid( gid );
  if ( node == 0 )
  {
    return kernel().model_manager.get_proxy_node( t, gid );
  }

  return node;
}

Node*
NodeManager::get_mpi_local_node_or_device_head( index gid )
{
  thread t = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp_for_gid( gid ) );

  Node* node = local_nodes_[ t ].get_node_by_gid( gid );

  if ( node == 0 )
  {
    return kernel().model_manager.get_proxy_node( t, gid );
  }
  if ( not node->has_proxies() )
  {
    node = local_nodes_[ 0 ].get_node_by_gid( gid );
  }

  return node;
}

std::vector< Node* >
NodeManager::get_thread_siblings( index gid ) const
{
  index num_threads = kernel().vp_manager.get_num_threads();
  std::vector< Node* > siblings( num_threads );
  for ( size_t t = 0; t < num_threads; ++t )
  {
    Node* node = local_nodes_[ t ].get_node_by_gid( gid );
    if ( node == 0 )
    {
      throw NoThreadSiblingsAvailable( gid );
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

#ifdef _OPENMP
#pragma omp critical( update_wfr_nodes_vec )
  {
// This code may be called from a thread-parallel context, when it is
// invoked by TargetIdentifierIndex::set_target() during parallel
// wiring. Nested OpenMP parallelism is problematic, therefore, we
// enforce single threading here. This should be unproblematic wrt
// performance, because the wfr_nodes_vec_ is rebuilt only once after
// changes in network size.
#endif

    // Check again, if the network size changed, since a previous thread
    // can have updated wfr_nodes_vec_ before.
    if ( size() != wfr_network_size_ )
    {

      /* We clear the existing wfr_nodes_vec_ and then rebuild it. */
      wfr_nodes_vec_.clear();
      wfr_nodes_vec_.resize( kernel().vp_manager.get_num_threads() );

      for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
      {
        wfr_nodes_vec_[ tid ].clear();

        size_t num_thread_local_wfr_nodes = 0;
        for ( size_t idx = 0; idx < local_nodes_[ tid ].size(); ++idx )
        {
          Node* node = local_nodes_[ tid ].get_node_by_index( idx );
          if ( node != 0 and node->node_uses_wfr_ )
          {
            ++num_thread_local_wfr_nodes;
          }
        }
        wfr_nodes_vec_[ tid ].reserve( num_thread_local_wfr_nodes );

        for ( size_t idx = 0; idx < local_nodes_[ tid ].size(); ++idx )
        {
          Node* node = local_nodes_[ tid ].get_node_by_index( idx );

          if ( node != 0 )
          {
            node->set_thread_lid( idx );
            if ( node->node_uses_wfr_ )
            {
              wfr_nodes_vec_[ tid ].push_back( node );
            }
          }
        }
      } // end of for threads

      wfr_network_size_ = size();

      // wfr_is_used_ indicates, whether at least one
      // of the threads has a neuron that uses waveform relaxtion
      // all threads then need to perform a wfr_update
      // step, because gather_events() has to be done in a
      // openmp single section
      wfr_is_used_ = false;
      for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
      {
        if ( wfr_nodes_vec_[ tid ].size() > 0 )
        {
          wfr_is_used_ = true;
        }
      }
    }
#ifdef _OPENMP
  } // end of omp critical region
#endif
}

void
NodeManager::destruct_nodes_()
{
#ifdef _OPENMP
#pragma omp parallel
  {
    index t = kernel().vp_manager.get_thread_id();
#else // clang-format off
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
#endif // clang-format on

    SparseNodeArray::const_iterator n;
    for ( n = local_nodes_[ t ].begin(); n != local_nodes_[ t ].end(); ++n )
    {
      // We call the destructor for each node excplicitly. This
      // destroys the objects without releasing their memory. Since
      // the Memory is owned by the Model objects, we must not call
      // delete on the Node objects!
      n->get_node()->~Node();
    }

    local_nodes_[ t ].clear();
  }
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
  n->init_buffers();
  n->calibrate();
}

void
NodeManager::prepare_nodes()
{
  assert( kernel().is_initialized() );

  /* We initialize the buffers of each node and calibrate it. */

  size_t num_active_nodes = 0;     // counts nodes that will be updated
  size_t num_active_wfr_nodes = 0; // counts nodes that use waveform relaxation

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised( kernel().vp_manager.get_num_threads() );

#ifdef _OPENMP
#pragma omp parallel reduction( + : num_active_nodes, num_active_wfr_nodes )
  {
    size_t t = kernel().vp_manager.get_thread_id();
#else
    for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    {
#endif

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

  } // end of parallel section / end of for threads

  // check if any exceptions have been raised
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
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
#ifdef _OPENMP
#pragma omp parallel
  {
    index t = kernel().vp_manager.get_thread_id();
#else // clang-format off
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
#endif // clang-format on
    SparseNodeArray::const_iterator n;
    for ( n = local_nodes_[ t ].begin(); n != local_nodes_[ t ].end(); ++n )
    {
      n->get_node()->post_run_cleanup();
    }
  }
}

/**
 * This function is called only if the thread data structures are properly set
 * up.
 */
void
NodeManager::finalize_nodes()
{
#ifdef _OPENMP
#pragma omp parallel
  {
    thread tid = kernel().vp_manager.get_thread_id();
#else // clang-format off
  for ( index tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
#endif // clang-format on
    SparseNodeArray::const_iterator n;
    for ( n = local_nodes_[ tid ].begin(); n != local_nodes_[ tid ].end(); ++n )
    {
      n->get_node()->finalize();
    }
  }
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
}

void
NodeManager::print( std::ostream& out ) const
{
  const index max_gid = size();
  const double max_gid_width = std::floor( std::log10( max_gid ) );
  const double gid_range_width = 6 + 2 * max_gid_width;

  for ( std::vector< modelrange >::const_iterator it = kernel().modelrange_manager.begin();
        it != kernel().modelrange_manager.end();
        ++it )
  {
    const index first_gid = it->get_first_gid();
    const index last_gid = it->get_last_gid();
    const Model* mod = kernel().model_manager.get_model( it->get_model_id() );

    std::stringstream gid_range_strs;
    gid_range_strs << std::setw( max_gid_width + 1 ) << first_gid;
    if ( last_gid != first_gid )
    {
      gid_range_strs << " .. " << std::setw( max_gid_width + 1 ) << last_gid;
    }
    out << std::setw( gid_range_width ) << std::left << gid_range_strs.str() << " " << mod->get_name();

    if ( it + 1 != kernel().modelrange_manager.end() )
    {
      out << std::endl;
    }
  }
}

void
NodeManager::set_status( index gid, const DictionaryDatum& d )
{
  for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    Node* node = local_nodes_[ t ].get_node_by_gid( gid );
    if ( node != 0 )
    {
      set_status_single_node_( *node, d );
    }
  }
}

void
NodeManager::get_status( DictionaryDatum& d )
{
  def< long >( d, names::network_size, size() );
}

void
NodeManager::set_status( const DictionaryDatum& d )
{
}
}
