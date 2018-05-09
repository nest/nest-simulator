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
#include "sibling_container.h"
#include "subnet.h"
#include "vp_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

NodeManager::NodeManager()
  : local_nodes_()
  , root_( 0 )
  , current_( 0 )
  , siblingcontainer_model_( 0 )
  , nodes_vec_()
  , wfr_nodes_vec_()
  , wfr_is_used_( false )
  , nodes_vec_network_size_( 0 ) // zero to force update
  , have_nodes_changed_( true )
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
  /*
   * TODO The code until the "END" comment below adds the root subnet.
   *      I am not sure this code should be here, it should certainly
   *      go into a properly named function.
   *
   * TODO It depends on ModelrangeManager being properly initialized.
   * TODO It depends on Base Models such as siblingcontainer are properly set
   *      up.
   *
   * We initialise the network with one subnet that is the root of the tree.
   * Note that we MUST NOT call add_node(), since it expects a properly
   * initialized network.
   */
  local_nodes_.reserve( 1 );
  kernel().modelrange_manager.add_range( 0, 0, 0 );

  // TODO The access to the base models below is maximally bad,
  //      and should be done in some elegant way vs ModelManager.
  // TODO formerly used pristine_models_ here - hope the models_ works to
  assert( kernel().model_manager.get_num_node_models() > 1 );

  Model* rootmodel = kernel().model_manager.get_model( 0 );
  assert( rootmodel != 0 );
  assert( rootmodel->get_name() == "subnet" );

  siblingcontainer_model_ = kernel().model_manager.get_model( 1 );
  assert( siblingcontainer_model_ != 0 );
  assert( siblingcontainer_model_->get_name() == "siblingcontainer" );

  SiblingContainer* root_container =
    static_cast< SiblingContainer* >( siblingcontainer_model_->allocate( 0 ) );
  local_nodes_.add_local_node( *root_container );
  root_container->reserve( kernel().vp_manager.get_num_threads() );
  root_container->set_model_id( -1 );

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    Node* newnode = rootmodel->allocate( tid );
    newnode->set_gid_( 0 );
    newnode->set_model_id( 0 );
    newnode->set_thread( tid );
    newnode->set_vp( kernel().vp_manager.thread_to_vp( tid ) );
    root_container->push_back( newnode );
  }

  current_ = root_ =
    static_cast< Subnet* >( ( *root_container ).get_thread_sibling( 0 ) );

  /* END of code adding the root subnet. */

  // explicitly force construction of nodes_vec_ to ensure consistent state
  nodes_vec_network_size_ = 0;
  ensure_valid_thread_local_ids();

  num_local_devices_ = 0;
}

void
NodeManager::finalize()
{
  destruct_nodes_();
}

void
NodeManager::reinit_nodes()
{
  /* Reinitialize state on all nodes, force init_buffers() on next
      call to simulate().
      Finding all nodes is non-trivial:
      - We iterate over local nodes only.
      - Nodes without proxies are not registered in nodes_. Instead, a
        SiblingContainer is created as container, and this container is
        stored in nodes_. The container then contains the actual nodes,
        which need to be reset.
      Thus, we iterate nodes_; additionally, we iterate the content of
      a Node if it's model id is -1, which indicates that it is a
      container.  Subnets are not iterated, since their nodes are
      registered in nodes_ directly.
    */
  for ( size_t n = 0; n < local_nodes_.size(); ++n )
  {
    Node* node = local_nodes_.get_node_by_index( n );
    assert( node != 0 );
    if ( node->num_thread_siblings() == 0 ) // not a SiblingContainer
    {
      node->init_state();
      node->set_buffers_initialized( false );
    }
    else if ( node->get_model_id() == -1 )
    {
      SiblingContainer* const c = dynamic_cast< SiblingContainer* >( node );
      assert( c );
      for ( std::vector< Node* >::iterator it = c->begin(); it != c->end();
            ++it )
      {
        ( *it )->init_state();
        ( *it )->set_buffers_initialized( false );
      }
    }
  }
}

DictionaryDatum
NodeManager::get_status( index idx )
{
  assert( idx != 0 );
  Node* target = get_node( idx );
  assert( target != 0 );

  DictionaryDatum d = target->get_status_base();

  return d;
}

index NodeManager::add_node( index mod, long n ) // no_p
{
  have_nodes_changed_ = true;

  assert( current_ != 0 );
  assert( root_ != 0 );

  if ( mod >= kernel().model_manager.get_num_node_models() )
  {
    throw UnknownModelID( mod );
  }

  if ( n < 1 )
  {
    throw BadProperty();
  }

  const thread n_threads = kernel().vp_manager.get_num_threads();
  assert( n_threads > 0 );

  const index min_gid = local_nodes_.get_max_gid() + 1;
  const index max_gid = min_gid + n;

  Model* model = kernel().model_manager.get_model( mod );
  assert( model != 0 );

  model->deprecation_warning( "Create" );

  /* current_ points to the instance of the current subnet on thread 0.
     The following code makes subnet a pointer to the wrapper container
     containing the instances of the current subnet on all threads.
   */
  const index subnet_gid = current_->get_gid();
  Node* subnet_node = local_nodes_.get_node_by_gid( subnet_gid );
  assert( subnet_node != 0 );

  SiblingContainer* subnet_container =
    dynamic_cast< SiblingContainer* >( subnet_node );
  assert( subnet_container != 0 );
  assert( subnet_container->num_thread_siblings()
    == static_cast< size_t >( n_threads ) );
  assert( subnet_container->get_thread_sibling( 0 ) == current_ );

  if ( max_gid > local_nodes_.max_size() or max_gid < min_gid )
  {
    LOG( M_ERROR,
      "NodeManager::add:node",
      "Requested number of nodes will overflow the memory." );
    LOG( M_ERROR, "NodeManager::add:node", "No nodes were created." );
    throw KernelException( "OutOfMemory" );
  }
  kernel().modelrange_manager.add_range( mod, min_gid, max_gid - 1 );

  if ( model->has_proxies() )
  {
    // In this branch we create nodes for all GIDs which are on a local thread
    const int n_per_process = n / kernel().mpi_manager.get_num_processes();
    const int n_per_thread = n_per_process / n_threads + 1;

    // We only need to reserve memory on the ranks on which we
    // actually create nodes.
    // TODO: This will work reasonably for round-robin. The extra 50 entries
    //       are for subnets and devices.
    local_nodes_.reserve( std::ceil( static_cast< double >( max_gid )
                            / kernel().mpi_manager.get_num_processes() ) + 50 );
    for ( thread tid = 0; tid < n_threads; ++tid )
    {
      // Model::reserve() reserves memory for n ADDITIONAL nodes on thread t
      // reserves at least one entry on each thread, nobody knows why
      model->reserve_additional( tid, n_per_thread );
    }

    size_t gid;
    if ( kernel().vp_manager.is_local_vp(
           kernel().vp_manager.suggest_vp_for_gid( min_gid ) ) )
    {
      gid = min_gid;
    }
    else
    {
      gid = next_local_gid_( min_gid );
    }
    size_t next_lid = current_->global_size() + gid - min_gid;
    // The next loop will not visit every node, if more than one rank is
    // present.
    // Since we already know what range of gids will be created, we can tell the
    // current subnet the range and subsequent calls to
    // `current_->add_remote_node()`
    // become irrelevant.
    current_->add_gid_range( min_gid, max_gid - 1 );

    // min_gid is first valid gid i should create, hence ask for the first local
    // gid after min_gid-1
    while ( gid < max_gid )
    {
      const thread vp = kernel().vp_manager.suggest_vp_for_gid( gid );
      const thread t = kernel().vp_manager.vp_to_thread( vp );

      if ( kernel().vp_manager.is_local_vp( vp ) )
      {
        Node* newnode = model->allocate( t );
        newnode->set_gid_( gid );
        newnode->set_model_id( mod );
        newnode->set_thread( t );
        newnode->set_vp( vp );

        local_nodes_.add_local_node( *newnode ); // put into local nodes list
        current_->add_node( newnode ); // and into current subnet, thread 0.

        // lid setting is wrong, if a range is set, as the subnet already
        // assumes,
        // the nodes are available.
        newnode->set_lid_( next_lid );
        const size_t next_gid = next_local_gid_( gid );
        next_lid += next_gid - gid;
        gid = next_gid;
      }
      else
      {
        ++gid; // brutal fix, next_lid has been set in if-branch
      }
    }
    // if last gid is not on this process, we need to add it as a remote node
    if ( not kernel().vp_manager.is_local_vp(
           kernel().vp_manager.suggest_vp_for_gid( max_gid - 1 ) ) )
    {
      local_nodes_.add_remote_node( max_gid - 1 ); // ensures max_gid is correct
      current_->add_remote_node( max_gid - 1, mod );
    }
  }
  else if ( not model->one_node_per_process() )
  {
    // We allocate space for n containers which will hold the threads
    // sorted. We use SiblingContainers to store the instances for
    // each thread to exploit the very efficient memory allocation for
    // nodes.
    //
    // These containers are registered in the global nodes_ array to
    // provide access to the instances both for manipulation by SLI
    // functions and so that NodeManager::calibrate() can discover the
    // instances and register them for updating.
    //
    // The instances are also registered with the instance of the
    // current subnet for the thread to which the created instance
    // belongs. This is mainly important so that the subnet structure
    // is preserved on all VPs.  Node enumeration is done on by the
    // registration with the per-thread instances.
    //
    // The wrapper container can be addressed under the GID assigned
    // to no-proxy node created. If this no-proxy node is NOT a
    // container (e.g. a device), then each instance can be retrieved
    // by giving the respective thread-id to get_node(). Instances of
    // SiblingContainers cannot be addressed individually.
    //
    // The allocation of the wrapper containers is spread over threads
    // to balance memory load.
    size_t container_per_thread = n / n_threads + 1;

    // since we create the n nodes on each thread, we reserve the full load.
    for ( thread tid = 0; tid < n_threads; ++tid )
    {
      model->reserve_additional( tid, n );
      siblingcontainer_model_->reserve_additional( tid, container_per_thread );
      static_cast< Subnet* >( subnet_container->get_thread_sibling( tid ) )
        ->reserve( n );
    }

    // The following loop creates n nodes. For each node, a wrapper is created
    // and filled with one instance per thread, in total n * n_thread nodes in
    // n wrappers.
    local_nodes_.reserve( std::ceil( static_cast< double >( max_gid )
                            / kernel().mpi_manager.get_num_processes() ) + 50 );
    for ( index gid = min_gid; gid < max_gid; ++gid )
    {
      const thread tid = kernel().vp_manager.vp_to_thread(
        kernel().vp_manager.suggest_vp_for_gid( gid ) );

      // keep track of number of local devices
      ++num_local_devices_;

      // Create wrapper and register with nodes_ array.
      SiblingContainer* container = static_cast< SiblingContainer* >(
        siblingcontainer_model_->allocate( tid ) );
      container->set_model_id(
        -1 ); // mark as pseudo-container wrapping replicas, see reset_network()
      container->reserve( n_threads ); // space for one instance per thread
      container->set_gid_( gid );
      local_nodes_.add_local_node( *container );

      // Generate one instance of desired model per thread
      for ( thread t = 0; t < n_threads; ++t )
      {
        Node* newnode = model->allocate( t );
        newnode->set_gid_( gid ); // all instances get the same global id.
        newnode->set_model_id( mod );
        newnode->set_thread( t );
        newnode->set_vp( kernel().vp_manager.thread_to_vp( t ) );
        newnode->set_local_device_id( num_local_devices_ - 1 );

        // Register instance with wrapper
        // container has one entry for each thread
        container->push_back( newnode );

        // Register instance with per-thread instance of enclosing subnet.
        static_cast< Subnet* >( subnet_container->get_thread_sibling( t ) )
          ->add_node( newnode );
      }
    }
  }
  else
  {
    // no proxies and one node per process
    // this is used by MUSIC proxies
    // Per r9700, this case is only relevant for music_*_proxy models,
    // which have a single instance per MPI process.
    for ( index gid = min_gid; gid < max_gid; ++gid )
    {
      // keep track of number of local devices
      ++num_local_devices_;

      Node* newnode = model->allocate( 0 );
      newnode->set_gid_( gid );
      newnode->set_model_id( mod );
      newnode->set_thread( 0 );
      newnode->set_vp( kernel().vp_manager.thread_to_vp( 0 ) );
      newnode->set_local_device_id( num_local_devices_ - 1 );

      // Register instance
      local_nodes_.add_local_node( *newnode );

      // and into current subnet, thread 0.
      current_->add_node( newnode );
    }
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

  // resize the target table for delivery of events to devices to make
  // sure the first dimension matches the number of local nodes and
  // the second dimension matches number of synapse types
  kernel()
    .connection_manager.resize_target_table_devices_to_number_of_neurons();
  kernel()
    .connection_manager
    .resize_target_table_devices_to_number_of_synapse_types();

  return max_gid - 1;
}

void
NodeManager::restore_nodes( const ArrayDatum& node_list )
{
  Subnet* root = get_cwn();
  const index gid_offset = size() - 1;
  Token* first = node_list.begin();
  const Token* end = node_list.end();
  if ( first == end )
  {
    return;
  }

  // We need to know the first and hopefully smallest GID to identify
  // if a parent is in or outside the range of restored nodes.
  // So we retrieve it here, from the first element of the node_list, assuming
  // that the node GIDs are in ascending order.
  DictionaryDatum node_props = getValue< DictionaryDatum >( *first );
  const index min_gid = ( *node_props )[ names::global_id ];

  for ( Token* node_t = first; node_t != end; ++node_t )
  {
    DictionaryDatum node_props = getValue< DictionaryDatum >( *node_t );
    std::string model_name = ( *node_props )[ names::model ];
    index model_id = kernel().model_manager.get_model_id( model_name.c_str() );
    index parent_gid = ( *node_props )[ names::parent ];
    index local_parent_gid = parent_gid;
    if ( parent_gid >= min_gid ) // if the parent is one of the restored nodes
    {
      local_parent_gid += gid_offset; // we must add the gid_offset
    }
    go_to( local_parent_gid );
    index node_gid = add_node( model_id );
    Node* node_ptr = get_node( node_gid );
    // we call directly set_status on the node
    // to bypass checking of unused dictionary items.
    node_ptr->set_status_base( node_props );
  }
  current_ = root;
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

inline index
NodeManager::next_local_gid_( index curr_gid ) const
{
  index rank = kernel().mpi_manager.get_rank();
  index procs = kernel().mpi_manager.get_num_processes();
  // responsible process for curr_gid
  index proc_of_curr_gid = curr_gid % procs;

  if ( proc_of_curr_gid == rank )
  {
    // I am responsible for curr_gid, then add 'modulo'.
    return curr_gid + procs;
  }
  else
  {
    // else add difference
    // make modulo positive and difference of my proc an curr_gid proc
    return curr_gid + ( procs + rank - proc_of_curr_gid ) % procs;
  }
}

index
NodeManager::get_max_num_local_nodes() const
{
  return static_cast< index >( ceil( static_cast< double >( size() )
    / kernel().vp_manager.get_num_virtual_processes() ) );
}

index
NodeManager::get_num_local_devices() const
{
  return num_local_devices_;
}

void
NodeManager::go_to( index n )
{
  if ( Subnet* target = dynamic_cast< Subnet* >( get_node( n ) ) )
  {
    current_ = target;
  }
  else
  {
    throw SubnetExpected();
  }
}

Node* NodeManager::get_node( index n, thread tid ) // no_p
{
  Node* node = local_nodes_.get_node_by_gid( n );
  if ( node == 0 )
  {
    return kernel().model_manager.get_proxy_node( tid, n );
  }

  if ( node->num_thread_siblings() == 0 )
  {
    return node; // plain node
  }

  if ( tid < 0 or tid >= static_cast< thread >( node->num_thread_siblings() ) )
  {
    throw UnknownNode();
  }

  return node->get_thread_sibling( tid );
}

const SiblingContainer*
NodeManager::get_thread_siblings( index n ) const
{
  Node* node = local_nodes_.get_node_by_gid( n );
  if ( node->num_thread_siblings() == 0 )
  {
    throw NoThreadSiblingsAvailable( n );
  }
  const SiblingContainer* siblings = dynamic_cast< SiblingContainer* >( node );
  assert( siblings != 0 );

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

      for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
      {
        nodes_vec_[ tid ].clear();
        wfr_nodes_vec_[ tid ].clear();

        // Loops below run from index 1, because index 0 is always the root
        // network, which is never updated.
        size_t num_thread_local_nodes = 0;
        size_t num_thread_local_wfr_nodes = 0;
        for ( size_t idx = 1; idx < local_nodes_.size(); ++idx )
        {
          Node* node = local_nodes_.get_node_by_index( idx );
          if ( not node->is_subnet()
            and ( node->get_thread() == tid
                  or node->num_thread_siblings() > 0 ) )
          {
            num_thread_local_nodes++;
            if ( node->node_uses_wfr() )
            {
              num_thread_local_wfr_nodes++;
            }
          }
        }
        nodes_vec_[ tid ].reserve( num_thread_local_nodes );
        wfr_nodes_vec_[ tid ].reserve( num_thread_local_wfr_nodes );

        for ( size_t idx = 1; idx < local_nodes_.size(); ++idx )
        {
          Node* node = local_nodes_.get_node_by_index( idx );

          // Subnets are never updated and therefore not included.
          if ( node->is_subnet() )
          {
            continue;
          }

          // If a node has thread siblings, it is a sibling container, and we
          // need to add the replica for the current thread. Otherwise, we have
          // a normal node, which is added only on the thread it belongs to.
          if ( node->num_thread_siblings() > 0 )
          {
            node->get_thread_sibling( tid )->set_thread_lid(
              nodes_vec_[ tid ].size() );
            nodes_vec_[ tid ].push_back( node->get_thread_sibling( tid ) );
          }
          else if ( node->get_thread() == tid )
          {
            // these nodes cannot be subnets
            node->set_thread_lid( nodes_vec_[ tid ].size() );
            nodes_vec_[ tid ].push_back( node );

            if ( node->node_uses_wfr() )
            {
              wfr_nodes_vec_[ tid ].push_back( node );
            }
          }
        }
      } // end of for threads

      nodes_vec_network_size_ = size();

      wfr_is_used_ = false;
      // wfr_is_used_ indicates, whether at least one
      // of the threads has a neuron that uses waveform relaxtion
      // all threads then need to perform a wfr_update
      // step, because gather_events() has to be done in a
      // openmp single section
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
  // We call the destructor for each node excplicitly. This destroys
  // the objects without releasing their memory. Since the Memory is
  // owned by the Model objects, we must not call delete on the Node
  // objects!
  for ( size_t n = 0; n < local_nodes_.size(); ++n )
  {
    Node* node = local_nodes_.get_node_by_index( n );
    assert( node != 0 );
    for ( size_t t = 0; t < node->num_thread_siblings(); ++t )
    {
      node->get_thread_sibling( t )->~Node();
    }
    node->~Node();
  }

  local_nodes_.clear();
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
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised.at( tid ).valid() )
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
    for ( size_t idx = 0; idx < local_nodes_.size(); ++idx )
    {
      Node* node = local_nodes_.get_node_by_index( idx );
      if ( node != 0 )
      {
        if ( node->num_thread_siblings() > 0 )
        {
          node->get_thread_sibling( t )->post_run_cleanup();
        }
        else
        {
          if ( static_cast< index >( node->get_thread() ) == t )
          {
            node->post_run_cleanup();
          }
        }
      }
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
    for ( size_t idx = 0; idx < local_nodes_.size(); ++idx )
    {
      Node* node = local_nodes_.get_node_by_index( idx );
      if ( node != 0 )
      {
        if ( node->num_thread_siblings() > 0 )
        {
          node->get_thread_sibling( tid )->finalize();
        }
        else
        {
          if ( node->get_thread() == tid )
          {
            node->finalize();
          }
        }
      }
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
  InstantaneousRateConnectionEvent::set_coeff_length(
    kernel().connection_manager.get_min_delay() );
  DelayedRateConnectionEvent::set_coeff_length(
    kernel().connection_manager.get_min_delay() );
  DiffusionConnectionEvent::set_coeff_length(
    kernel().connection_manager.get_min_delay() );
  TimeDrivenSpikeEvent::set_coeff_length(
    kernel().connection_manager.get_min_delay() );
}

void
NodeManager::print( index p, int depth )
{
  Subnet* target = dynamic_cast< Subnet* >( get_node( p ) );
  if ( target != NULL )
  {
    std::cout << target->print_network( depth + 1, 0 );
  }
  else
  {
    throw SubnetExpected();
  }
}


void
NodeManager::set_status( index gid, const DictionaryDatum& d )
{

  assert( gid > 0 or "This function cannot be called for the root node." );
  if ( gid > 0 )
  {
    // we first handle normal nodes, except the root (GID 0)
    Node* target = local_nodes_.get_node_by_gid( gid );
    if ( target != 0 )
    {
      // node is local
      if ( target->num_thread_siblings() == 0 )
      {
        set_status_single_node_( *target, d );
      }
      else
      {
        for ( size_t t = 0; t < target->num_thread_siblings(); ++t )
        {
          // non-root container for devices without proxies and subnets
          // we iterate over all threads
          assert( target->get_thread_sibling( t ) != 0 );
          set_status_single_node_( *( target->get_thread_sibling( t ) ), d );
        }
      }
    }
    return;
  }
}

void
NodeManager::get_status( DictionaryDatum& d )
{
  def< long >( d, names::network_size, size() );

  std::map< long, size_t > sna_cts = local_nodes_.get_step_ctr();
  DictionaryDatum cdict( new Dictionary );
  for ( std::map< long, size_t >::const_iterator cit = sna_cts.begin();
        cit != sna_cts.end();
        ++cit )
  {
    std::stringstream s;
    s << cit->first;
    ( *cdict )[ s.str() ] = cit->second;
  }
}

void
NodeManager::set_status( const DictionaryDatum& d )
{
  std::string tmp;
  // proceed only if there are unaccessed items left
  if ( not d->all_accessed( tmp ) )
  {
    // Fetch the target pointer here. We cannot do it above, since
    // Network::set_status() may modify the root compound if the number
    // of threads changes. HEP, 2008-10-20
    Node* target = local_nodes_.get_node_by_gid( 0 );
    assert( target != 0 );

    for ( size_t t = 0; t < target->num_thread_siblings(); ++t )
    {
      // Root container for per-thread subnets. We must prevent clearing of
      // access flags before each compound's properties are set by passing false
      // as last arg we iterate over all threads
      assert( target->get_thread_sibling( t ) != 0 );
      set_status_single_node_( *( target->get_thread_sibling( t ) ), d, false );
    }
  }
}

void
NodeManager::reset_nodes_state()
{
  /* Reinitialize state on all nodes, force init_buffers() on next
     call to simulate().
     Finding all nodes is non-trivial:
     - We iterate over local nodes only.
     - Nodes without proxies are not registered in nodes_. Instead, a
       SiblingContainer is created as container, and this container is
       stored in nodes_. The container then contains the actual nodes,
       which need to be reset.
     Thus, we iterate nodes_; additionally, we iterate the content of
     a Node if it's model id is -1, which indicates that it is a
     container.  Subnets are not iterated, since their nodes are
     registered in nodes_ directly.
   */
  for ( size_t n = 0; n < local_nodes_.size(); ++n )
  {
    Node* node = local_nodes_.get_node_by_index( n );
    assert( node != 0 );
    if ( node->num_thread_siblings() == 0 ) // not a SiblingContainer
    {
      node->init_state();
      node->set_buffers_initialized( false );
    }
    else if ( node->get_model_id() == -1 )
    {
      SiblingContainer* const c = dynamic_cast< SiblingContainer* >( node );
      assert( c );
      for ( std::vector< Node* >::iterator it = c->begin(); it != c->end();
            ++it )
      {
        ( *it )->init_state();
        ( *it )->set_buffers_initialized( false );
      }
    }
  }
}
}
