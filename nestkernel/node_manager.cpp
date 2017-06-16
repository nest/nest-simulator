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
#include "genericmodel_impl.h"
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
  , nodes_vec_()
  , wfr_nodes_vec_()
  , wfr_is_used_( false )
  , nodes_vec_network_size_( 0 ) // zero to force update
  , num_active_nodes_( 0 )
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
  // explicitly force construction of nodes_vec_ to ensure consistent state
  nodes_vec_network_size_ = 0;
  local_nodes_.resize( kernel().vp_manager.get_num_threads() );
  ensure_valid_thread_local_ids();
}

void
NodeManager::finalize()
{
  destruct_nodes_();
}

void
NodeManager::reinit_nodes()
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
      // Reinitialize state on all nodes, forcing init_buffers() on
      // next call to simulate().
      n->get_node()->init_state();
      n->get_node()->set_buffers_initialized( false );
    }
  }
}

DictionaryDatum
NodeManager::get_status( index idx )
{
  Node* target = get_node( idx );
  assert( target != 0 );

  DictionaryDatum d = target->get_status_base();

  return d;
}

GIDCollectionPTR
NodeManager::add_node( index model_id, long n )
{
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

  // TODO480: We should reconsider if it is really smart that max_gid is
  // the largest new gid PLUS ONE. This leads to -1 in several places below
  // and is a bit confusing.
  const index min_gid = local_nodes_.at( 0 ).get_max_gid() + 1;
  const index max_gid = min_gid + n;
  if ( max_gid > local_nodes_.at( 0 ).max_size() or max_gid < min_gid )
  {
    LOG( M_ERROR,"NodeManager::add_node",
	 "Requested number of nodes will overflow the memory. "
	 "No nodes were created");
    throw KernelException( "OutOfMemory" );
  }

  kernel().modelrange_manager.add_range( model_id, min_gid, max_gid - 1 );

  // Nodes are created in parallel by all threads. The algorithm is
  // the same for all types of nodes:
  // 1. reserve additional space in the thread-local SparseNodeArray
  // 2. reserve additional space in the corresponding thread-local
  //    model pool
  // 3. create the node and set its properties
  // 4. register the new node in the thread-local SparseNodeArray
  //
  // In order to guarantee a consistent representation, we set the new
  // maximum gid on all SparseNodeArrays.
  //
  // We need to distinguish three cases for the creation:
  // 1. For nodes having proxies (usually neurons), we only create
  //    a node on the thread responsible for the node. On all other
  //    threads, we just do nothing. get_node() will take care of
  //    returning the corresponding proxy for non-existing nodes.
  // 2. For nodes without proxies and with replicas on each threads
  //    (normal devices) we create a node on each thread.
  // 3. For nodes without proxies and only one instance per process
  //    (MUSIC proxies) we create a node only on thread 0.

  // TODO: put the code in the blocks below in three functions with the
  //       following signatures in node_manager.h:
  // void add_node_neurons_( Model* model, index min_gid, index max_gid );
  // void add_node_devices_( Model* model, index min_gid, index max_gid );
  // void add_node_music_( Model* model, index min_gid, index max_gid );

  if ( model->has_proxies() )
  {
    // add_node_neurons_( model, min_gid, max_gid );

	// upper limit for number of neurons per thread; in practice, either
	// max_new_per_thread-1 or max_new_per_thread nodes will be created
    const size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
    const size_t max_new_per_thread = static_cast< size_t >( std::ceil(
    		static_cast< double >( max_gid - min_gid ) / num_vps ) );

#ifdef _OPENMP
#pragma omp parallel
#endif
    {
      const index t = kernel().vp_manager.get_thread_id();
      // TODO480: We should move more of the reservation logic into the
      // SparseNodeArray. We should tell SNA only max_gid-1 and whether the model
      // needs local replicas or not. Then SNA can manage memory. This can reduce
      // bloat due to round-up when using many Create calls for >1 thread
      local_nodes_.at( t ).reserve_additional( max_new_per_thread );
      model->reserve_additional( t, max_new_per_thread );

      const thread vp = kernel().vp_manager.thread_to_vp( t );
      index gid = min_gid;
      const thread vp_of_gid = kernel().vp_manager.suggest_vp( gid );

      // TODO480: Refactor next_vp_local_gid_ so that we can initialize gid as
      //          index gid = next_vp_local_gid_( min_gid, vp );
      if ( vp != vp_of_gid )
      {
        gid = next_vp_local_gid_( gid, vp );
      }

      while ( gid < max_gid )
      {
        Node* node = model->allocate( t );
        node->set_gid_( gid );
        node->set_model_id( model->get_model_id() );
        node->set_thread( t );
        node->set_vp( vp );

        local_nodes_[ t ].add_local_node( *node );
	    gid = next_vp_local_gid_( gid, vp );
      }
    }
  }
  else if ( not model->one_node_per_process() )
  {
    // add_node_devices_( model, min_gid, max_gid );

    int n_per_thread = ( max_gid - min_gid ) + 1;

#ifdef _OPENMP
#pragma omp parallel
    {
      index t = kernel().vp_manager.get_thread_id();
#else // clang-format off
    for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    {
#endif // clang-format on
      local_nodes_[ t ].reserve_additional( n_per_thread );
      model->reserve_additional( t, n_per_thread );

      for ( index gid = min_gid; gid < max_gid; ++gid )
      {
        Node* node = model->allocate( t );
        node->set_gid_( gid );
        node->set_model_id( model->get_model_id() );
        node->set_thread( t );
        node->set_vp( kernel().vp_manager.thread_to_vp( t ) );

        local_nodes_[ t ].add_local_node( *node );
      }
    }
  }
  else
  {
    // add_node_music_( model, min_gid, max_gid );

    for ( index gid = min_gid; gid < max_gid; ++gid )
    {
      Node* node = model->allocate( 0 );
      node->set_gid_( gid );
      node->set_model_id( model->get_model_id() );
      node->set_thread( 0 );
      node->set_vp( kernel().vp_manager.thread_to_vp( 0 ) );

      local_nodes_[ 0 ].add_local_node( *node );
    }
  }
  
#ifdef _OPENMP
#pragma omp parallel
#endif
  {
    const size_t t = kernel().vp_manager.get_thread_id();
    local_nodes_.at( t ).update_max_gid( max_gid-1 );
  }

  // set off-grid spike communication if necessary
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

  return GIDCollectionPTR(
    new GIDCollectionPrimitive( min_gid, max_gid - 1, model_id ) );
}

inline index
NodeManager::next_vp_local_gid_( index gid, thread vp ) const
{
  thread vp_of_gid = kernel().vp_manager.suggest_vp( gid );
  thread num_vp = kernel().vp_manager.get_num_virtual_processes();

  if ( vp < vp_of_gid )
  {
    return gid + num_vp - vp_of_gid;
  }
  if ( vp > vp_of_gid )
  {
    return gid + vp - vp_of_gid;
  }

  return gid + num_vp;
 }

void
NodeManager::restore_nodes( const ArrayDatum& node_list )
{
  Token* first = node_list.begin();
  const Token* end = node_list.end();
  if ( first == end )
  {
    return;
  }

  for ( Token* node_t = first; node_t != end; ++node_t )
  {
    DictionaryDatum node_props = getValue< DictionaryDatum >( *node_t );
    std::string model_name = ( *node_props )[ names::model ];
    index model_id = kernel().model_manager.get_model_id( model_name.c_str() );
    GIDCollectionPTR node = add_node( model_id );
    Node* node_ptr = get_node( ( *node->begin() ).gid );
    // we call directly set_status on the node
    // to bypass checking of unused dictionary items.
    node_ptr->set_status_base( node_props );
  }
}

void
NodeManager::init_state( index GID )
{
  Node* n = get_node( GID );
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

// TODO480: I wonder if the need to loop over threads can be a performance
// bottleneck. BUT: I think we only need to check whether the node is local
// TO THE THREAD, and then we do not need to loop.
bool
NodeManager::is_local_gid( index gid ) const
{
  bool is_local = false;
  thread num_threads = kernel().vp_manager.get_num_threads();
  for ( thread t = 0; t < num_threads; ++t )
  {
    is_local |= local_nodes_[ t ].get_node_by_gid( gid ) != 0;
  }
  return is_local;
}

Node* NodeManager::get_node( index gid, thread t )
{
  assert( 0 <= t and t < kernel().vp_manager.get_num_threads() );

  Node* node = local_nodes_[ t ].get_node_by_gid( gid );
  if ( node == 0 )
  {
    return kernel().model_manager.get_proxy_node( t, gid );
  }

  return node;
}

Node* NodeManager::get_local_thread_node( index gid, thread t )
{
  Node* node = local_nodes_[ t ].get_node_by_gid( gid );

  return node;
}

std::vector< Node* >
NodeManager::get_thread_siblings( index gid ) const
{
  thread num_threads =  kernel().vp_manager.get_num_threads();
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
  if ( size() == nodes_vec_network_size_ )
  {
    return;
  }

#ifdef _OPENMP
#pragma omp critical( update_nodes_vec )
  {
// This code may be called from a thread-parallel context, when it is
// invoked by TargetIdentifierIndex::set_target() during parallel
// wiring. Nested OpenMP parallelism is problematic, therefore, we
// enforce single threading here. This should be unproblematic wrt
// performance, because the nodes_vec_ is rebuilt only once after
// changes in network size.
#endif

    // Check again, if the network size changed, since a previous thread
    // can have updated nodes_vec_ before.
    if ( size() != nodes_vec_network_size_ )
    {

      /* We clear the existing nodes_vec_ and then rebuild it. */
      nodes_vec_.clear();
      nodes_vec_.resize( kernel().vp_manager.get_num_threads() );
      wfr_nodes_vec_.clear();
      wfr_nodes_vec_.resize( kernel().vp_manager.get_num_threads() );

      for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
      {
        nodes_vec_[ t ].clear();
        wfr_nodes_vec_[ t ].clear();

        size_t num_thread_local_nodes = 0;
        size_t num_thread_local_wfr_nodes = 0;
        for ( size_t idx = 0; idx < local_nodes_[ t ].size(); ++idx )
        {
          Node* node = local_nodes_[ t ].get_node_by_index( idx );
	      if ( node != 0 )
	      {
            ++num_thread_local_nodes;
            if ( node->node_uses_wfr() )
            {
              ++num_thread_local_wfr_nodes;
            }
          }
        }
        nodes_vec_[ t ].reserve( num_thread_local_nodes );
        wfr_nodes_vec_[ t ].reserve( num_thread_local_wfr_nodes );

        for ( size_t idx = 0; idx < local_nodes_[ t ].size(); ++idx )
        {
          Node* node = local_nodes_[ t ].get_node_by_index( idx );
	      if ( node != 0 )
	      {
            node->set_thread_lid( nodes_vec_[ t ].size() );
            nodes_vec_[ t ].push_back( node );
            if ( node->node_uses_wfr() )
            {
              wfr_nodes_vec_[ t ].push_back( node );
            }
	      }
        }
      } // end of for threads

      nodes_vec_network_size_ = size();

      // wfr_is_used_ indicates, whether at least one
      // of the threads has a neuron that uses waveform relaxtion
      // all threads then need to perform a wfr_update
      // step, because gather_events() has to be done in a
      // openmp single section
      wfr_is_used_ = false;
      for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
      {
        if ( wfr_nodes_vec_[ t ].size() > 0 )
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
NodeManager::set_status_single_node_( Node& target,
  const DictionaryDatum& d,
  bool clear_flags )
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
    ALL_ENTRIES_ACCESSED(
      *d, "NodeManager::set_status", "Unread dictionary entries: " );
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

  std::vector< lockPTR< WrappedThreadException > > exceptions_raised(
    kernel().vp_manager.get_num_threads() );

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
      for ( std::vector< Node* >::iterator it = nodes_vec_[ t ].begin();
            it != nodes_vec_[ t ].end();
            ++it )
      {
        prepare_node_( *it );
        if ( not( *it )->is_frozen() )
        {
          ++num_active_nodes;
          if ( ( *it )->node_uses_wfr() )
          {
            ++num_active_wfr_nodes;
          }
        }
      }
    }
    catch ( std::exception& e )
    {
      // so throw the exception after parallel region
      exceptions_raised.at( t ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( e ) );
    }

  } // end of parallel section / end of for threads

  // check if any exceptions have been raised
  for ( index thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
  {
    if ( exceptions_raised.at( thr ).valid() )
    {
      throw WrappedThreadException( *( exceptions_raised.at( thr ) ) );
    }
  }

  std::ostringstream os;
  std::string tmp_str = num_active_nodes == 1 ? " node" : " nodes";
  os << "Preparing " << num_active_nodes << tmp_str << " for simulation.";

  if ( num_active_wfr_nodes != 0 )
  {
    tmp_str = num_active_wfr_nodes == 1 ? " uses " : " use ";
    os << " " << num_active_wfr_nodes << " of them" << tmp_str
       << "iterative solution techniques.";
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
    index t = kernel().vp_manager.get_thread_id();
#else // clang-format off
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
#endif // clang-format on
    SparseNodeArray::const_iterator n;
    for ( n = local_nodes_[ t ].begin(); n != local_nodes_[ t ].end(); ++n )
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
    kernel().connection_manager.get_min_delay()
    * ( kernel().simulation_manager.get_wfr_interpolation_order() + 1 ) );
}

void
NodeManager::print( std::ostream& out ) const
{
  const index max_gid = size();
  const double max_gid_width = std::floor( std::log10( max_gid ) );
  const double gid_range_width = 6 + 2 * max_gid_width;

  for ( std::vector< modelrange >::const_iterator it =
          kernel().modelrange_manager.begin();
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
    out << std::setw( gid_range_width ) << std::left << gid_range_strs.str()
        << " " << mod->get_name();

    if ( it + 1 != kernel().modelrange_manager.end() )
    {
      out << std::endl;
    }
  }
}

void
NodeManager::set_status( index gid, const DictionaryDatum& d )
{
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
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
