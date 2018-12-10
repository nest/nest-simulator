/*
 *  conn_builder.cpp
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

#include "conn_builder.h"

// C++ includes:
#include <set>

// Includes from libnestutil:
#include "logging.h"

// Includes from librandom:
#include "binomial_randomdev.h"
#include "gsl_binomial_randomdev.h"
#include "gslrandomgen.h"
#include "normal_randomdev.h"

// Includes from nestkernel:
#include "conn_builder_impl.h"
#include "conn_parameter.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_names.h"
#include "node.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dict.h"
#include "fdstream.h"
#include "name.h"

nest::ConnBuilder::ConnBuilder( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : sources_( &sources )
  , targets_( &targets )
  , autapses_( true )
  , multapses_( true )
  , make_symmetric_( false )
  , creates_symmetric_connections_( false )
  , exceptions_raised_( kernel().vp_manager.get_num_threads() )
  , synapse_model_id_( kernel().model_manager.get_synapsedict()->lookup(
      "static_synapse" ) )
  , weight_( 0 )
  , delay_( 0 )
  , param_dicts_()
  , parameters_requiring_skipping_()
{
  // read out rule-related parameters -------------------------
  //  - /rule has been taken care of above
  //  - rule-specific params are handled by subclass c'tor
  updateValue< bool >( conn_spec, names::autapses, autapses_ );
  updateValue< bool >( conn_spec, names::multapses, multapses_ );
  updateValue< bool >( conn_spec, names::make_symmetric, make_symmetric_ );

  // read out synapse-related parameters ----------------------
  if ( not syn_spec->known( names::model ) )
  {
    throw BadProperty( "Synapse spec must contain synapse model." );
  }
  const std::string syn_name = ( *syn_spec )[ names::model ];
  if ( not kernel().model_manager.get_synapsedict()->known( syn_name ) )
  {
    throw UnknownSynapseType( syn_name );
  }

  synapse_model_id_ =
    kernel().model_manager.get_synapsedict()->lookup( syn_name );

  // We need to make sure that Connect can process all synapse parameters
  // specified.
  const ConnectorModel& synapse_model =
    kernel().model_manager.get_synapse_prototype( synapse_model_id_, 0 );
  synapse_model.check_synapse_params( syn_spec );

  DictionaryDatum syn_defaults =
    kernel().model_manager.get_connector_defaults( synapse_model_id_ );

  // All synapse models have the possibility to set the delay (see
  // SynIdDelay), but some have homogeneous weights, hence it should
  // be possible to set the delay without the weight.
  default_weight_ = not syn_spec->known( names::weight );

  default_delay_ = not syn_spec->known( names::delay );

  // If neither weight nor delay are given in the dict, we handle this
  // separately. Important for hom_w synapses, on which weight cannot
  // be set. However, we use default weight and delay for _all_ types
  // of synapses.
  default_weight_and_delay_ = ( default_weight_ and default_delay_ );

#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during
  // connection setup
  ( *syn_defaults )[ names::music_channel ] = 0;
#endif

  if ( not default_weight_and_delay_ )
  {
    weight_ = syn_spec->known( names::weight )
      ? ConnParameter::create( ( *syn_spec )[ names::weight ],
          kernel().vp_manager.get_num_threads() )
      : ConnParameter::create( ( *syn_defaults )[ names::weight ],
          kernel().vp_manager.get_num_threads() );
    register_parameters_requiring_skipping_( *weight_ );
    delay_ = syn_spec->known( names::delay )
      ? ConnParameter::create(
          ( *syn_spec )[ names::delay ], kernel().vp_manager.get_num_threads() )
      : ConnParameter::create( ( *syn_defaults )[ names::delay ],
          kernel().vp_manager.get_num_threads() );
  }
  else if ( default_weight_ )
  {
    delay_ = syn_spec->known( names::delay )
      ? ConnParameter::create(
          ( *syn_spec )[ names::delay ], kernel().vp_manager.get_num_threads() )
      : ConnParameter::create( ( *syn_defaults )[ names::delay ],
          kernel().vp_manager.get_num_threads() );
  }
  register_parameters_requiring_skipping_( *delay_ );

  // Structural plasticity parameters
  // Check if both pre and post synaptic element are provided
  if ( syn_spec->known( names::pre_synaptic_element )
    and syn_spec->known( names::post_synaptic_element ) )
  {
    pre_synaptic_element_name_ =
      getValue< std::string >( syn_spec, names::pre_synaptic_element );
    post_synaptic_element_name_ =
      getValue< std::string >( syn_spec, names::post_synaptic_element );

    use_pre_synaptic_element_ = true;
    use_post_synaptic_element_ = true;
  }
  else
  {
    if ( syn_spec->known( names::pre_synaptic_element )
      or syn_spec->known( names::post_synaptic_element ) )
    {
      throw BadProperty(
        "In order to use structural plasticity, both a pre and post synaptic "
        "element must be specified" );
    }

    use_pre_synaptic_element_ = false;
    use_post_synaptic_element_ = false;
  }

  // synapse-specific parameters
  // TODO: Can we create this set once and for all?
  //       Should not be done as static initialization, since
  //       that might conflict with static initialization of
  //       Name system.
  std::set< Name > skip_set;
  skip_set.insert( names::weight );
  skip_set.insert( names::delay );
  skip_set.insert( names::min_delay );
  skip_set.insert( names::max_delay );
  skip_set.insert( names::num_connections );
  skip_set.insert( names::synapse_model );

  for ( Dictionary::const_iterator default_it = syn_defaults->begin();
        default_it != syn_defaults->end();
        ++default_it )
  {
    const Name param_name = default_it->first;
    if ( skip_set.find( param_name ) != skip_set.end() )
    {
      continue; // weight, delay or not-settable parameter
    }

    if ( syn_spec->known( param_name ) )
    {
      synapse_params_[ param_name ] = ConnParameter::create(
        ( *syn_spec )[ param_name ], kernel().vp_manager.get_num_threads() );
      register_parameters_requiring_skipping_( *synapse_params_[ param_name ] );
    }
  }

  // Now create dictionary with dummy values that we will use
  // to pass settings to the synapses created. We create it here
  // once to avoid re-creating the object over and over again.
  if ( synapse_params_.size() > 0 )
  {
    for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
    {
      param_dicts_.push_back( new Dictionary() );

      for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
            it != synapse_params_.end();
            ++it )
      {
        if ( it->first == names::receptor_type
          or it->first == names::music_channel
          or it->first == names::synapse_label )
        {
          ( *param_dicts_[ tid ] )[ it->first ] =
            Token( new IntegerDatum( 0 ) );
        }
        else
        {
          ( *param_dicts_[ tid ] )[ it->first ] =
            Token( new DoubleDatum( 0.0 ) );
        }
      }
    }
  }

  // If make_symmetric_ is requested call reset on all parameters in order
  // to check if all parameters support symmetric connections
  if ( make_symmetric_ )
  {
    if ( weight_ )
    {
      weight_->reset();
    }
    if ( delay_ )
    {
      delay_->reset();
    }
    for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
          it != synapse_params_.end();
          ++it )
    {
      it->second->reset();
    }
  }
}


nest::ConnBuilder::~ConnBuilder()
{
  delete weight_;
  delete delay_;
  for ( std::map< Name, ConnParameter* >::iterator it = synapse_params_.begin();
        it != synapse_params_.end();
        ++it )
  {
    delete it->second;
  }
}

/**
 * Updates the number of connected synaptic elements in the
 * target and the source.
 * Returns 0 if the target is either on another
 * MPI machine or another thread. Returns 1 otherwise.
 *
 * @param sgid id of the source
 * @param tgid id of the target
 * @param tid thread id
 * @param update amount of connected synaptic elements to update
 * @return
 */
bool
nest::ConnBuilder::change_connected_synaptic_elements( index sgid,
  index tgid,
  const thread tid,
  int update )
{

  int local = true;
  // check whether the source is on this mpi machine
  if ( kernel().node_manager.is_local_gid( sgid ) )
  {
    Node* const source = kernel().node_manager.get_node( sgid, tid );
    const thread source_thread = source->get_thread();

    // check whether the source is on our thread
    if ( tid == source_thread )
    {
      // update the number of connected synaptic elements
      source->connect_synaptic_element( pre_synaptic_element_name_, update );
    }
  }

  // check whether the target is on this mpi machine
  if ( not kernel().node_manager.is_local_gid( tgid ) )
  {
    local = false;
  }
  else
  {
    Node* const target = kernel().node_manager.get_node( tgid, tid );
    const thread target_thread = target->get_thread();
    // check whether the target is on our thread
    if ( tid != target_thread )
    {
      local = false;
    }
    else
    {
      // update the number of connected synaptic elements
      target->connect_synaptic_element( post_synaptic_element_name_, update );
    }
  }
  return local;
}

/**
 * Now we can connect with or without structural plasticity
 */
void
nest::ConnBuilder::connect()
{
  // We test here, and not in the ConnBuilder constructor, so the derived
  // classes are fully constructed when the test is executed
  if ( kernel().model_manager.connector_requires_symmetric( synapse_model_id_ )
    and not( is_symmetric() or make_symmetric_ ) )
  {
    throw BadProperty(
      "Connections with this synapse model can only be created as "
      "one-to-one connections with \"make_symmetric\" set to true "
      "or as all-to-all connections with equal source and target "
      "populations and default or scalar parameters." );
  }

  if ( make_symmetric_ and not supports_symmetric() )
  {
    throw NotImplemented(
      "This connection rule does not support symmetric connections." );
  }

  if ( use_structural_plasticity_() )
  {
    if ( make_symmetric_ )
    {
      throw NotImplemented(
        "Symmetric connections are not supported in combination with "
        "structural plasticity." );
    }
    sp_connect_();
  }
  else
  {
    connect_();
    if ( make_symmetric_ and not creates_symmetric_connections_ )
    {
      // call reset on all parameters
      if ( weight_ )
      {
        weight_->reset();
      }
      if ( delay_ )
      {
        delay_->reset();
      }
      for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
            it != synapse_params_.end();
            ++it )
      {
        it->second->reset();
      }

      std::swap( sources_, targets_ );
      connect_();
      std::swap( sources_, targets_ ); // re-establish original state
    }
  }

  // check if any exceptions have been raised
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised_.at( tid ).valid() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( tid ) ) );
    }
  }
}

/**
 * Now we can delete synapses with or without structural plasticity
 */
void
nest::ConnBuilder::disconnect()
{
  if ( use_structural_plasticity_() )
  {
    sp_disconnect_();
  }
  else
  {
    disconnect_();
  }

  // check if any exceptions have been raised
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised_.at( tid ).valid() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( tid ) ) );
    }
  }
}

void
nest::ConnBuilder::single_connect_( index sgid,
  Node& target,
  thread target_thread,
  librandom::RngPtr& rng )
{
  if ( this->requires_proxies() and not target.has_proxies() )
  {
    throw IllegalConnection(
      "Cannot use this rule to connect to nodes"
      " without proxies (usually devices)." );
  }

  if ( param_dicts_.empty() ) // indicates we have no synapse params
  {
    const DictionaryDatum params = new Dictionary; // empty parameter dictionary
    // required by connect() calls

    if ( default_weight_and_delay_ )
    {
      kernel().connection_manager.connect(
        sgid, &target, target_thread, synapse_model_id_, params );
    }
    else if ( default_weight_ )
    {
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        params,
        delay_->value_double( target_thread, rng ) );
    }
    else if ( default_delay_ )
    {
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        params,
        numerics::nan,
        weight_->value_double( target_thread, rng ) );
    }
    else
    {
      double delay = delay_->value_double( target_thread, rng );
      double weight = weight_->value_double( target_thread, rng );
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        params,
        delay,
        weight );
    }
  }
  else
  {
    assert( kernel().vp_manager.get_num_threads()
      == static_cast< thread >( param_dicts_.size() ) );

    for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
          it != synapse_params_.end();
          ++it )
    {
      if ( it->first == names::receptor_type
        or it->first == names::music_channel
        or it->first == names::synapse_label )
      {
        try
        {
          // change value of dictionary entry without allocating new datum
          IntegerDatum* id = static_cast< IntegerDatum* >(
            ( ( *param_dicts_[ target_thread ] )[ it->first ] ).datum() );
          ( *id ) = it->second->value_int( target_thread, rng );
        }
        catch ( KernelException& e )
        {
          if ( it->first == names::receptor_type )
          {
            throw BadProperty( "Receptor type must be of type integer." );
          }
          else if ( it->first == names::music_channel )
          {
            throw BadProperty( "Music channel type must be of type integer." );
          }
          else if ( it->first == names::synapse_label )
          {
            throw BadProperty( "Synapse label must be of type integer." );
          }
        }
      }
      else
      {
        // change value of dictionary entry without allocating new datum
        DoubleDatum* dd = static_cast< DoubleDatum* >(
          ( ( *param_dicts_[ target_thread ] )[ it->first ] ).datum() );
        ( *dd ) = it->second->value_double( target_thread, rng );
      }
    }

    if ( default_weight_and_delay_ )
    {
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        param_dicts_[ target_thread ] );
    }
    else if ( default_weight_ )
    {
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        param_dicts_[ target_thread ],
        delay_->value_double( target_thread, rng ) );
    }
    else if ( default_delay_ )
    {
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        param_dicts_[ target_thread ],
        numerics::nan,
        weight_->value_double( target_thread, rng ) );
    }
    else
    {
      double delay = delay_->value_double( target_thread, rng );
      double weight = weight_->value_double( target_thread, rng );
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_id_,
        param_dicts_[ target_thread ],
        delay,
        weight );
    }
  }
}

void
nest::ConnBuilder::set_pre_synaptic_element_name( const std::string& name )
{
  if ( name.empty() )
  {
    throw BadProperty( "pre_synaptic_element cannot be empty." );
  }

  pre_synaptic_element_name_ = Name( name );
  use_pre_synaptic_element_ = not name.empty();
}

void
nest::ConnBuilder::set_post_synaptic_element_name( const std::string& name )
{
  if ( name.empty() )
  {
    throw BadProperty( "post_synaptic_element cannot be empty." );
  }

  post_synaptic_element_name_ = Name( name );
  use_post_synaptic_element_ = not name.empty();
}

bool
nest::ConnBuilder::all_parameters_scalar_() const
{
  bool all_scalar = true;
  if ( weight_ )
  {
    all_scalar = all_scalar and weight_->is_scalar();
  }
  if ( delay_ )
  {
    all_scalar = all_scalar and delay_->is_scalar();
  }
  for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
        it != synapse_params_.end();
        ++it )
  {
    all_scalar = all_scalar and it->second->is_scalar();
  }
  return all_scalar;
}

bool
nest::ConnBuilder::loop_over_targets_() const
{
  return targets_->size() < kernel().node_manager.local_nodes_size()
    or not targets_->is_range() or parameters_requiring_skipping_.size() > 0;
}

nest::OneToOneBuilder::OneToOneBuilder( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
{
  // make sure that target and source population have the same size
  if ( sources_->size() != targets_->size() )
  {
    throw DimensionMismatch(
      "Source and Target population must be of the same size." );
  }
}

void
nest::OneToOneBuilder::connect_()
{

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      const size_t expected_targets =
        std::ceil( targets_->size()
          / static_cast< double >(
                     kernel().vp_manager.get_num_virtual_processes() ) );

      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      if ( loop_over_targets_() )
      {
        for ( GIDCollection::const_iterator tgid = targets_->begin(),
                                            sgid = sources_->begin();
              tgid != targets_->end();
              ++tgid, ++sgid )
        {
          assert( sgid != sources_->end() );

          if ( *sgid == *tgid and not autapses_ )
          {
            continue;
          }

          // check whether the target is on this mpi machine
          if ( not kernel().node_manager.is_local_gid( *tgid ) )
          {
            skip_conn_parameter_( tid );
            continue;
          }

          Node* const target = kernel().node_manager.get_node( *tgid, tid );
          const thread target_thread = target->get_thread();

          // check whether the target is on our thread
          if ( tid != target_thread )
          {
            skip_conn_parameter_( tid );
            continue;
          }

          single_connect_( *sgid, *target, target_thread, rng );
        }
      }
      else
      {
        for ( SparseNodeArray::const_iterator it =
                kernel().node_manager.local_nodes_begin();
              it != kernel().node_manager.local_nodes_end();
              ++it )
        {
          Node* const target = ( *it ).get_node();
          const thread target_thread = target->get_thread();

          if ( tid != target_thread )
          {
            // no skipping required / possible,
            // as we iterate only over local nodes
            continue;
          }

          const index tgid = ( *it ).get_gid();
          const int idx = targets_->find( tgid );
          if ( idx < 0 ) // Is local node in target list?
          {
            continue;
          }

          // one-to-one, thus we can use target idx for source as well
          const index sgid = ( *sources_ )[ idx ];
          if ( not autapses_ and sgid == tgid )
          {
            // no skipping required / possible,
            // as we iterate only over local nodes
            continue;
          }

          single_connect_( sgid, *target, target_thread, rng );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

/**
 * Solves the disconnection of two nodes on a OneToOne basis without
 * structural plasticity. This means this method can be manually called
 * by the user to delete existing synapses.
 */
void
nest::OneToOneBuilder::disconnect_()
{

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      for ( GIDCollection::const_iterator tgid = targets_->begin(),
                                          sgid = sources_->begin();
            tgid != targets_->end();
            ++tgid, ++sgid )
      {

        assert( sgid != sources_->end() );

        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
        {
          // Disconnecting: no parameter skipping required
          continue;
        }

        Node* const target = kernel().node_manager.get_node( *tgid, tid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
        {
          // Disconnecting: no parameter skipping required
          continue;
        }
        single_disconnect_( *sgid, *target, target_thread );
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

/**
 * Solves the connection of two nodes on a OneToOne basis with
 * structural plasticity. This means this method is used by the
 * structural plasticity manager based on the homostatic rules defined
 * for the synaptic elements on each node.
 */
void
nest::OneToOneBuilder::sp_connect_()
{

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      for ( GIDCollection::const_iterator tgid = targets_->begin(),
                                          sgid = sources_->begin();
            tgid != targets_->end();
            ++tgid, ++sgid )
      {
        assert( sgid != sources_->end() );

        if ( *sgid == *tgid and not autapses_ )
        {
          continue;
        }

        if ( not change_connected_synaptic_elements( *sgid, *tgid, tid, 1 ) )
        {
          skip_conn_parameter_( tid );
          continue;
        }
        Node* const target = kernel().node_manager.get_node( *tgid, tid );
        const thread target_thread = target->get_thread();

        if ( tid == target_thread )
        {
          single_connect_( *sgid, *target, target_thread, rng );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

/**
 * Solves the disconnection of two nodes on a OneToOne basis with
 * structural plasticity. This means this method is used by the
 * structural plasticity manager based on the homostatic rules defined
 * for the synaptic elements on each node.
 */
void
nest::OneToOneBuilder::sp_disconnect_()
{

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      for ( GIDCollection::const_iterator tgid = targets_->begin(),
                                          sgid = sources_->begin();
            tgid != targets_->end();
            ++tgid, ++sgid )
      {
        assert( sgid != sources_->end() );

        if ( not change_connected_synaptic_elements( *sgid, *tgid, tid, -1 ) )
        {
          // Disconnecting: no parameter skipping required
          continue;
        }
        Node* const target = kernel().node_manager.get_node( *tgid, tid );
        const thread target_thread = target->get_thread();

        single_disconnect_( *sgid, *target, target_thread );
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

void
nest::AllToAllBuilder::connect_()
{

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      const size_t expected_targets =
        std::ceil( sources_->size() * targets_->size()
          / static_cast< double >(
                     kernel().vp_manager.get_num_virtual_processes() ) );

      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      if ( loop_over_targets_() )
      {
        for ( GIDCollection::const_iterator tgid = targets_->begin();
              tgid != targets_->end();
              ++tgid )
        {
          // check whether the target is on this mpi machine
          if ( not kernel().node_manager.is_local_gid( *tgid ) )
          {
            skip_conn_parameter_( tid, sources_->size() );
            continue;
          }

          Node* const target = kernel().node_manager.get_node( *tgid, tid );

          inner_connect_( tid, rng, target, *tgid, true );
        }
      }
      else
      {
        for ( SparseNodeArray::const_iterator it =
                kernel().node_manager.local_nodes_begin();
              it != kernel().node_manager.local_nodes_end();
              ++it )
        {
          Node* const target = ( *it ).get_node();
          const index tgid = ( *it ).get_gid();

          // Is the local node in the targets list?
          if ( targets_->find( tgid ) < 0 )
          {
            continue;
          }

          inner_connect_( tid, rng, target, tgid, false );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

void
nest::AllToAllBuilder::inner_connect_( const int tid,
  librandom::RngPtr& rng,
  Node* target,
  index tgid,
  bool skip )
{
  const thread target_thread = target->get_thread();

  // check whether the target is on our thread
  if ( tid != target_thread )
  {
    if ( skip )
    {
      skip_conn_parameter_( tid, sources_->size() );
    }
    return;
  }

  for ( GIDCollection::const_iterator sgid = sources_->begin();
        sgid != sources_->end();
        ++sgid )
  {
    if ( not autapses_ and *sgid == tgid )
    {
      if ( skip )
      {
        skip_conn_parameter_( target_thread );
      }
      continue;
    }

    single_connect_( *sgid, *target, target_thread, rng );
  }
}

/**
 * Solves the connection of two nodes on a AllToAll basis with
 * structural plasticity. This means this method is used by the
 * structural plasticity manager based on the homostatic rules defined
 * for the synaptic elements on each node.
 */
void
nest::AllToAllBuilder::sp_connect_()
{
#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();
    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        for ( GIDCollection::const_iterator sgid = sources_->begin();
              sgid != sources_->end();
              ++sgid )
        {
          if ( not autapses_ and *sgid == *tgid )
          {
            skip_conn_parameter_( tid );
            continue;
          }
          if ( not change_connected_synaptic_elements( *sgid, *tgid, tid, 1 ) )
          {
            skip_conn_parameter_( tid, sources_->size() );
            continue;
          }
          Node* const target = kernel().node_manager.get_node( *tgid, tid );
          const thread target_thread = target->get_thread();
          single_connect_( *sgid, *target, target_thread, rng );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

/**
 * Solves the disconnection of two nodes on a AllToAll basis without
 * structural plasticity. This means this method can be manually called
 * by the user to delete existing synapses.
 */
void
nest::AllToAllBuilder::disconnect_()
{
#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
        {
          // Disconnecting: no parameter skipping required
          continue;
        }

        Node* const target = kernel().node_manager.get_node( *tgid, tid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
        {
          // Disconnecting: no parameter skipping required
          continue;
        }

        for ( GIDCollection::const_iterator sgid = sources_->begin();
              sgid != sources_->end();
              ++sgid )
        {
          single_disconnect_( *sgid, *target, target_thread );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

/**
 * Solves the disconnection of two nodes on a AllToAll basis with
 * structural plasticity. This means this method is used by the
 * structural plasticity manager based on the homostatic rules defined
 * for the synaptic elements on each node.
 */
void
nest::AllToAllBuilder::sp_disconnect_()
{
#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        for ( GIDCollection::const_iterator sgid = sources_->begin();
              sgid != sources_->end();
              ++sgid )
        {
          if ( not change_connected_synaptic_elements( *sgid, *tgid, tid, -1 ) )
          {
            // Disconnecting: no parameter skipping required
            continue;
          }
          Node* const target = kernel().node_manager.get_node( *tgid, tid );
          const thread target_thread = target->get_thread();
          single_disconnect_( *sgid, *target, target_thread );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

nest::FixedInDegreeBuilder::FixedInDegreeBuilder( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
  , indegree_( ( *conn_spec )[ names::indegree ] )
{
  // check for potential errors
  long n_sources = static_cast< long >( sources_->size() );
  if ( n_sources == 0 )
  {
    throw BadProperty( "Source array must not be empty." );
  }
  // verify that indegree is not larger than source population if multapses are
  // disabled
  if ( not multapses_ )
  {
    if ( indegree_ > n_sources )
    {
      throw BadProperty( "Indegree cannot be larger than population size." );
    }
    else if ( indegree_ == n_sources and not autapses_ )
    {
      LOG( M_WARNING,
        "FixedInDegreeBuilder::connect",
        "Multapses and autapses prohibited. When the sources and the targets "
        "have a non-empty "
        "intersection, the connect algorithm will enter an infinite loop." );
      return;
    }

    if ( indegree_ > 0.9 * n_sources )
    {
      LOG( M_WARNING,
        "FixedInDegreeBuilder::connect",
        "Multapses are prohibited and you request more than 90% connectivity. "
        "Expect long connecting times!" );
    }
  } // if (not multapses_ )

  if ( indegree_ < 0 )
  {
    throw BadProperty( "Indegree cannot be less than zero." );
  }
}

void
nest::FixedInDegreeBuilder::connect_()
{
#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      const size_t expected_targets =
        std::ceil( targets_->size()
          / static_cast< double >(
                     kernel().vp_manager.get_num_virtual_processes() ) );

      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      if ( loop_over_targets_() )
      {
        for ( GIDCollection::const_iterator tgid = targets_->begin();
              tgid != targets_->end();
              ++tgid )
        {
          // check whether the target is on this mpi machine
          if ( not kernel().node_manager.is_local_gid( *tgid ) )
          {
            // skip array parameters handled in other virtual processes
            skip_conn_parameter_( tid, indegree_ );
            continue;
          }

          Node* target = kernel().node_manager.get_node( *tgid, tid );

          inner_connect_( tid, rng, target, *tgid, true );
        }
      }
      else
      {
        for ( SparseNodeArray::const_iterator it =
                kernel().node_manager.local_nodes_begin();
              it != kernel().node_manager.local_nodes_end();
              ++it )
        {
          Node* const target = ( *it ).get_node();
          const index tgid = ( *it ).get_gid();

          // Is the local node in the targets list?
          if ( targets_->find( tgid ) < 0 )
          {
            continue;
          }

          inner_connect_( tid, rng, target, tgid, false );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}

void
nest::FixedInDegreeBuilder::inner_connect_( const int tid,
  librandom::RngPtr& rng,
  Node* target,
  index tgid,
  bool skip )
{
  const thread target_thread = target->get_thread();

  // check whether the target is on our thread
  if ( tid != target_thread )
  {
    // skip array parameters handled in other virtual processes
    if ( skip )
    {
      skip_conn_parameter_( tid, indegree_ );
    }
    return;
  }

  std::set< long > ch_ids;
  long n_rnd = sources_->size();

  for ( long j = 0; j < indegree_; ++j )
  {
    unsigned long s_id;
    index sgid;

    do
    {
      s_id = rng->ulrand( n_rnd );
      sgid = ( *sources_ )[ s_id ];
    } while ( ( not autapses_ and sgid == tgid )
      or ( not multapses_ and ch_ids.find( s_id ) != ch_ids.end() ) );

    if ( not multapses_ )
    {
      ch_ids.insert( s_id );
    }

    single_connect_( sgid, *target, target_thread, rng );
  }
}

nest::FixedOutDegreeBuilder::FixedOutDegreeBuilder(
  const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
  , outdegree_( ( *conn_spec )[ names::outdegree ] )
{
  // check for potential errors
  long n_targets = static_cast< long >( targets_->size() );
  if ( n_targets == 0 )
  {
    throw BadProperty( "Target array must not be empty." );
  }

  // verify that outdegree is not larger than target population if multapses are
  // disabled
  if ( not multapses_ )
  {
    if ( outdegree_ > n_targets )
    {
      throw BadProperty( "Outdegree cannot be larger than population size." );
    }
    else if ( outdegree_ == n_targets and not autapses_ )
    {
      LOG( M_WARNING,
        "FixedOutDegreeBuilder::connect",
        "Multapses and autapses prohibited. When the sources and the targets "
        "have a non-empty "
        "intersection, the connect algorithm will enter an infinite loop." );
      return;
    }

    if ( outdegree_ > 0.9 * n_targets )
    {
      LOG( M_WARNING,
        "FixedOutDegreeBuilder::connect",
        "Multapses are prohibited and you request more than 90% connectivity. "
        "Expect long connecting times!" );
    }
  }

  if ( outdegree_ < 0 )
  {
    throw BadProperty( "Outdegree cannot be less than zero." );
  }
}

void
nest::FixedOutDegreeBuilder::connect_()
{
  librandom::RngPtr grng = kernel().rng_manager.get_grng();

  for ( GIDCollection::const_iterator sgid = sources_->begin();
        sgid != sources_->end();
        ++sgid )
  {
    std::set< long > ch_ids;
    std::vector< index > tgt_ids_;
    const long n_rnd = targets_->size();

    for ( long j = 0; j < outdegree_; ++j )
    {
      unsigned long t_id;
      index tgid;

      do
      {
        t_id = grng->ulrand( n_rnd );
        tgid = ( *targets_ )[ t_id ];
      } while ( ( not autapses_ and tgid == *sgid )
        or ( not multapses_ and ch_ids.find( t_id ) != ch_ids.end() ) );

      if ( not multapses_ )
      {
        ch_ids.insert( t_id );
      }

      tgt_ids_.push_back( tgid );
    }

#pragma omp parallel
    {
      // get thread id
      const thread tid = kernel().vp_manager.get_thread_id();

      try
      {
        const size_t expected_new_syns =
          std::ceil( sources_->size() * outdegree_
            / static_cast< double >(
                       kernel().vp_manager.get_num_virtual_processes() ) );

        // allocate pointer to thread specific random generator
        librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

        for ( std::vector< index >::const_iterator tgid = tgt_ids_.begin();
              tgid != tgt_ids_.end();
              ++tgid )
        {
          // check whether the target is on this mpi machine
          if ( not kernel().node_manager.is_local_gid( *tgid ) )
          {
            // skip array parameters handled in other virtual processes
            skip_conn_parameter_( tid );
            continue;
          }

          Node* const target = kernel().node_manager.get_node( *tgid, tid );
          const thread target_thread = target->get_thread();

          // check whether the target is on our thread
          if ( tid != target_thread )
          {
            // skip array parameters handled in other virtual processes
            skip_conn_parameter_( tid );
            continue;
          }

          single_connect_( *sgid, *target, target_thread, rng );
        }
      }
      catch ( std::exception& err )
      {
        // We must create a new exception here, err's lifetime ends at
        // the end of the catch block.
        exceptions_raised_.at( tid ) = lockPTR< WrappedThreadException >(
          new WrappedThreadException( err ) );
      }
    }
  }
}

nest::FixedTotalNumberBuilder::FixedTotalNumberBuilder(
  const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
  , N_( ( *conn_spec )[ names::N ] )
{

  // check for potential errors

  // verify that total number of connections is not larger than
  // N_sources*N_targets
  if ( not multapses_ )
  {
    if ( ( N_ > static_cast< long >( sources_->size() * targets_->size() ) ) )
    {
      throw BadProperty(
        "Total number of connections cannot exceed product "
        "of source and targer population sizes." );
    }
  }

  if ( N_ < 0 )
  {
    throw BadProperty( "Total number of connections cannot be negative." );
  }

  // for now multapses cannot be forbidden
  // TODO: Implement option for multapses_ = False, where already existing
  // connections are stored in
  // a bitmap
  if ( not multapses_ )
  {
    throw NotImplemented(
      "Connect doesn't support the suppression of multapses in the "
      "FixedTotalNumber connector." );
  }
}

void
nest::FixedTotalNumberBuilder::connect_()
{
  const int M = kernel().vp_manager.get_num_virtual_processes();
  const long size_sources = sources_->size();
  const long size_targets = targets_->size();

  // drawing connection ids

  // Compute the distribution of targets over processes using the modulo
  // function
  std::vector< size_t > number_of_targets_on_vp( M, 0 );
  std::vector< index > local_targets;
  local_targets.reserve(
    size_targets / kernel().mpi_manager.get_num_processes() );
  for ( size_t t = 0; t < targets_->size(); t++ )
  {
    int vp = kernel().vp_manager.suggest_vp_for_gid( ( *targets_ )[ t ] );
    ++number_of_targets_on_vp[ vp ];
    if ( kernel().vp_manager.is_local_vp( vp ) )
    {
      local_targets.push_back( ( *targets_ )[ t ] );
    }
  }

  // We use the multinomial distribution to determine the number of
  // connections that will be made on one virtual process, i.e. we
  // partition the set of edges into n_vps subsets. The number of
  // edges on one virtual process is binomially distributed with
  // the boundary condition that the sum of all edges over virtual
  // processes is the total number of edges.
  // To obtain the num_conns_on_vp we adapt the gsl
  // implementation of the multinomial distribution.

  // K from gsl is equivalent to M = n_vps
  // N is already taken from stack
  // p[] is targets_on_vp
  std::vector< long > num_conns_on_vp( M, 0 ); // corresponds to n[]

  // calculate exact multinomial distribution
  // get global rng that is tested for synchronization for all threads
  librandom::RngPtr grng = kernel().rng_manager.get_grng();

  // HEP: instead of counting upwards, we might count remaining_targets and
  // remaining_partitions down. why?
  // begin code adapted from gsl 1.8 //
  double sum_dist = 0.0; // corresponds to sum_p
  // norm is equivalent to size_targets
  unsigned int sum_partitions = 0; // corresponds to sum_n
// substituting gsl_ran call
#ifdef HAVE_GSL
  librandom::GSL_BinomialRandomDev bino( grng, 0, 0 );
#else
  librandom::BinomialRandomDev bino( grng, 0, 0 );
#endif

  for ( int k = 0; k < M; k++ )
  {
    if ( number_of_targets_on_vp[ k ] > 0 )
    {
      double num_local_targets =
        static_cast< double >( number_of_targets_on_vp[ k ] );
      double p_local = num_local_targets / ( size_targets - sum_dist );
      bino.set_p( p_local );
      bino.set_n( N_ - sum_partitions );
      num_conns_on_vp[ k ] = bino.ldev();
    }

    sum_dist += static_cast< double >( number_of_targets_on_vp[ k ] );
    sum_partitions += static_cast< unsigned int >( num_conns_on_vp[ k ] );
  }

// end code adapted from gsl 1.8

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      const int vp_id = kernel().vp_manager.thread_to_vp( tid );

      if ( kernel().vp_manager.is_local_vp( vp_id ) )
      {
        librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

        // gather local target gids
        std::vector< index > thread_local_targets;
        thread_local_targets.reserve( number_of_targets_on_vp[ vp_id ] );
        for ( std::vector< index >::const_iterator it = local_targets.begin();
              it != local_targets.end();
              ++it )
        {
          if ( kernel().vp_manager.suggest_vp_for_gid( *it ) == vp_id )
          {
            thread_local_targets.push_back( *it );
          }
        }
        assert(
          thread_local_targets.size() == number_of_targets_on_vp[ vp_id ] );

        while ( num_conns_on_vp[ vp_id ] > 0 )
        {

          // draw random numbers for source node from all source neurons
          const long s_index = rng->ulrand( size_sources );
          // draw random numbers for target node from
          // targets_on_vp on this virtual process
          const long t_index = rng->ulrand( thread_local_targets.size() );
          // map random number of source node to gid corresponding to
          // the source_adr vector
          const long sgid = ( *sources_ )[ s_index ];
          // map random number of target node to gid using the
          // targets_on_vp vector
          const long tgid = thread_local_targets[ t_index ];

          Node* const target = kernel().node_manager.get_node( tgid, tid );
          const thread target_thread = target->get_thread();

          if ( autapses_ or sgid != tgid )
          {
            single_connect_( sgid, *target, target_thread, rng );
            num_conns_on_vp[ vp_id ]--;
          }
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}


nest::BernoulliBuilder::BernoulliBuilder( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
  , p_( ( *conn_spec )[ names::p ] )
{
  if ( p_ < 0 or 1 < p_ )
  {
    throw BadProperty( "Connection probability 0 <= p <= 1 required." );
  }
}


void
nest::BernoulliBuilder::connect_()
{
#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    // compute expected number of connections from binomial
    // distribution; estimate an upper bound by assuming Gaussianity
    const size_t max_num_connections =
      std::ceil( float( targets_->size() ) * float( sources_->size() )
        / kernel().vp_manager.get_num_virtual_processes() );

    const size_t expected_num_connections = max_num_connections * p_;
    const size_t std_num_connections =
      std::sqrt( max_num_connections * p_ * ( 1 - p_ ) );

    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      if ( loop_over_targets_() )
      {
        for ( GIDCollection::const_iterator tgid = targets_->begin();
              tgid != targets_->end();
              ++tgid )
        {
          // check whether the target is on this mpi machine
          if ( not kernel().node_manager.is_local_gid( *tgid ) )
          {
            continue;
          }

          Node* const target = kernel().node_manager.get_node( *tgid, tid );

          inner_connect_( tid, rng, target, *tgid );
        }
      }

      else
      {
        for ( SparseNodeArray::const_iterator it =
                kernel().node_manager.local_nodes_begin();
              it != kernel().node_manager.local_nodes_end();
              ++it )
        {
          Node* const target = ( *it ).get_node();
          const index tgid = ( *it ).get_gid();

          // Is the local node in the targets list?
          if ( targets_->find( tgid ) < 0 )
          {
            continue;
          }

          inner_connect_( tid, rng, target, tgid );
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  } // of omp parallel
}

void
nest::BernoulliBuilder::inner_connect_( const int tid,
  librandom::RngPtr& rng,
  Node* target,
  index tgid )
{
  const thread target_thread = target->get_thread();

  // check whether the target is on our thread
  if ( tid != target_thread )
  {
    return;
  }

  // It is not possible to create multapses with this type of BernoulliBuilder,
  // hence leave out corresponding checks.

  for ( GIDCollection::const_iterator sgid = sources_->begin();
        sgid != sources_->end();
        ++sgid )
  {
    if ( not autapses_ and *sgid == tgid )
    {
      continue;
    }

    if ( rng->drand() >= p_ )
    {
      continue;
    }

    single_connect_( *sgid, *target, target_thread, rng );
  }
}


nest::SymmetricBernoulliBuilder::SymmetricBernoulliBuilder(
  const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
  , p_( ( *conn_spec )[ names::p ] )
{
  // This connector takes care of symmetric connections on its own
  creates_symmetric_connections_ = true;

  if ( p_ < 0 or 1 <= p_ )
  {
    throw BadProperty( "Connection probability 0 <= p < 1 required." );
  }

  if ( not multapses_ )
  {
    throw BadProperty( "Multapses must be enabled." );
  }

  if ( autapses_ )
  {
    throw BadProperty( "Autapses must be disabled." );
  }

  if ( not make_symmetric_ )
  {
    throw BadProperty( "Symmetric connections must be enabled." );
  }
}


void
nest::SymmetricBernoulliBuilder::connect_()
{
  // Allocate a pointer to the global random generator. This is used to create a
  // random generator for each thread, each using the same seed obtained from
  // the global rng, making all threads across all processes generate identical
  // random number streams. This is required to generate symmetric connections:
  // if we would loop only over local targets, we might miss the symmetric
  // counterpart to a connection where a local target is chosen as a source.
  librandom::RngPtr grng = kernel().rng_manager.get_grng();
  const unsigned long s =
    grng->ulrand( std::numeric_limits< unsigned int >::max() );

#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();

// Create a random generator for each thread, each using the same seed obtained
// from the global rng. This ensures that all threads across all processes
// generate identical random number streams.
#ifdef HAVE_GSL
    librandom::RngPtr rng(
      new librandom::GslRandomGen( gsl_rng_knuthran2002, s ) );
#else
    librandom::RngPtr rng = librandom::RandomGen::create_knuthlfg_rng( s );
#endif

    try
    {
#ifdef HAVE_GSL
      librandom::GSL_BinomialRandomDev bino( rng, 0, 0 );
#else
      librandom::BinomialRandomDev bino( rng, 0, 0 );
#endif
      bino.set_p( p_ );
      bino.set_n( sources_->size() );

      // compute expected number of connections from binomial
      // distribution; estimate an upper bound by assuming Gaussianity
      const size_t max_num_connections =
        std::ceil( targets_->size() * sources_->size()
          / static_cast< double >(
                     kernel().vp_manager.get_num_virtual_processes() ) );
      const size_t expected_num_connections = max_num_connections * p_;
      const size_t std_num_connections =
        std::sqrt( max_num_connections * p_ * ( 1 - p_ ) );

      unsigned long indegree;
      index sgid;
      std::set< index > previous_sgids;
      Node* target;
      thread target_thread;
      Node* source;
      thread source_thread;

      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        // sample indegree according to truncated Binomial distribution
        indegree = sources_->size();
        while ( indegree >= sources_->size() )
        {
          indegree = bino.ldev();
        }
        assert( indegree < sources_->size() );

        // check whether the target is on this thread
        if ( kernel().node_manager.is_local_gid( *tgid ) )
        {
          target = kernel().node_manager.get_node( *tgid, tid );
          target_thread = target->get_thread();
        }
        else
        {
          target = NULL;
          target_thread = invalid_thread_;
        }

        previous_sgids.clear();

        // choose indegree number of sources randomly from all sources
        size_t i = 0;
        while ( i < indegree )
        {
          sgid = ( *sources_ )[ rng->ulrand( sources_->size() ) ];

          // Avoid autapses and multapses. Due to symmetric connectivity,
          // multapses might exist if the target neuron with gid sgid draws the
          // source with gid tgid while choosing sources itself.
          if ( sgid == *tgid
            or previous_sgids.find( sgid ) != previous_sgids.end() )
          {
            continue;
          }
          previous_sgids.insert( sgid );

          if ( kernel().node_manager.is_local_gid( sgid ) )
          {
            source = kernel().node_manager.get_node( sgid, tid );
            source_thread = source->get_thread();
          }
          else
          {
            source = NULL;
            source_thread = invalid_thread_;
          }

          // if target is local: connect
          if ( target_thread == tid )
          {
            assert( target != NULL );
            single_connect_( sgid, *target, target_thread, rng );
          }

          // if source is local: connect
          if ( source_thread == tid )
          {
            assert( source != NULL );
            single_connect_( *tgid, *source, source_thread, rng );
          }

          ++i;
        }
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}


/**
 * The SPBuilder is in charge of the creation of synapses during the simulation
 * under the control of the structural plasticity manager
 * @param net the network
 * @param sources the source nodes on which synapses can be created/deleted
 * @param targets the target nodes on which synapses can be created/deleted
 * @param conn_spec connectivity specs
 * @param syn_spec synapse specs
 */
nest::SPBuilder::SPBuilder( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
{
  // Check that both pre and post synaptic element are provided
  if ( not use_pre_synaptic_element_ or not use_post_synaptic_element_ )
  {
    throw BadProperty(
      "pre_synaptic_element and/or post_synaptic_elements is missing" );
  }
}

void
nest::SPBuilder::update_delay( delay& d ) const
{
  if ( get_default_delay() )
  {
    DictionaryDatum syn_defaults =
      kernel().model_manager.get_connector_defaults( get_synapse_model() );
    d = Time( Time::ms( getValue< double >( syn_defaults, "delay" ) ) )
          .get_steps();
  }
}

void
nest::SPBuilder::sp_connect( GIDCollection sources, GIDCollection targets )
{
  connect_( sources, targets );

  // check if any exceptions have been raised
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised_.at( tid ).valid() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( tid ) ) );
    }
  }
}

void
nest::SPBuilder::connect_()
{
  throw NotImplemented(
    "Connection without structural plasticity is not possible for this "
    "connection builder" );
}

/**
 * In charge of dynamically creating the new synapses
 * @param sources nodes from which synapses can be created
 * @param targets target nodes for the newly created synapses
 */
void
nest::SPBuilder::connect_( GIDCollection sources, GIDCollection targets )
{
  // Code copied and adapted from OneToOneBuilder::connect_()
  // make sure that target and source population have the same size
  if ( sources.size() != targets.size() )
  {
    LOG( M_ERROR,
      "Connect",
      "Source and Target population must be of the same size." );
    throw DimensionMismatch();
  }

#pragma omp parallel
  {
    // get thread id
    const thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      for ( GIDCollection::const_iterator tgid = targets.begin(),
                                          sgid = sources.begin();
            tgid != targets.end();
            ++tgid, ++sgid )
      {
        assert( sgid != sources.end() );

        if ( *sgid == *tgid and not autapses_ )
        {
          continue;
        }

        if ( not change_connected_synaptic_elements( *sgid, *tgid, tid, 1 ) )
        {
          skip_conn_parameter_( tid );
          continue;
        }
        Node* const target = kernel().node_manager.get_node( *tgid, tid );
        const thread target_thread = target->get_thread();

        single_connect_( *sgid, *target, target_thread, rng );
      }
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) =
        lockPTR< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }
}
