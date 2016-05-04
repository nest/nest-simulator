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
  , symmetric_( false )
  , exceptions_raised_( kernel().vp_manager.get_num_threads() )
  , synapse_model_( kernel().model_manager.get_synapsedict()->lookup(
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
  updateValue< bool >( conn_spec, names::symmetric, symmetric_ );

  // read out synapse-related parameters ----------------------
  if ( !syn_spec->known( names::model ) )
    throw BadProperty( "Synapse spec must contain synapse model." );
  const std::string syn_name = ( *syn_spec )[ names::model ];
  if ( not kernel().model_manager.get_synapsedict()->known( syn_name ) )
    throw UnknownSynapseType( syn_name );

  // if another synapse than static_synapse is defined we need to make
  // sure that Connect can process all parameter specified
  if ( syn_name != "static_synapse" )
    check_synapse_params_( syn_name, syn_spec );

  synapse_model_ = kernel().model_manager.get_synapsedict()->lookup( syn_name );

  DictionaryDatum syn_defaults =
    kernel().model_manager.get_connector_defaults( synapse_model_ );

  // All synapse models have the possibility to set the delay (see
  // SynIdDelay), but some have homogeneous weights, hence it should
  // be possible to set the delay without the weight.
  default_weight_ = !syn_spec->known( names::weight );

  default_delay_ = !syn_spec->known( names::delay );

  // If neither weight nor delay are given in the dict, we handle this
  // separately. Important for hom_w synapses, on which weight cannot
  // be set. However, we use default weight and delay for _all_ types
  // of synapses.
  default_weight_and_delay_ = ( default_weight_ && default_delay_ );

#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during
  // connection setup
  ( *syn_defaults )[ names::music_channel ] = 0;
#endif

  if ( !default_weight_and_delay_ )
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
    && syn_spec->known( names::post_synaptic_element ) )
  {
    pre_synaptic_element_name =
      getValue< std::string >( syn_spec, names::pre_synaptic_element );
    post_synaptic_element_name =
      getValue< std::string >( syn_spec, names::post_synaptic_element );
  }
  else
  {
    if ( syn_spec->known( names::pre_synaptic_element )
      || syn_spec->known( names::post_synaptic_element ) )
    {
      throw BadProperty(
        "In order to use structural plasticity, both a pre and post synaptic "
        "element must be specified" );
    }
    pre_synaptic_element_name = "";
    post_synaptic_element_name = "";
  }

  // synapse-specific parameters
  // TODO: Can we create this set once and for all?
  //       Should not be done as static initialization, since
  //       that might conflict with static initialization of
  //       Name system.
  std::set< Name > skip_set;
  skip_set.insert( names::weight );
  skip_set.insert( names::delay );
  skip_set.insert( Name( "min_delay" ) );
  skip_set.insert( Name( "max_delay" ) );
  skip_set.insert( Name( "num_connections" ) );
  skip_set.insert( Name( "num_connectors" ) );
  skip_set.insert( Name( "property_object" ) );
  skip_set.insert( Name( "synapsemodel" ) );

  for ( Dictionary::const_iterator default_it = syn_defaults->begin();
        default_it != syn_defaults->end();
        ++default_it )
  {
    const Name param_name = default_it->first;
    if ( skip_set.find( param_name ) != skip_set.end() )
      continue; // weight, delay or not-settable parameter

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
    for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    {
      param_dicts_.push_back( new Dictionary() );

      for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
            it != synapse_params_.end();
            ++it )
      {
        if ( it->first == names::receptor_type
          || it->first == names::music_channel
          || it->first == names::synapse_label )
          ( *param_dicts_[ t ] )[ it->first ] = Token( new IntegerDatum( 0 ) );
        else
          ( *param_dicts_[ t ] )[ it->first ] = Token( new DoubleDatum( 0.0 ) );
      }
    }
  }

  // If symmetric_ is requested call reset on all parameters in order
  // to check if all parameters support symmetric connections
  if ( symmetric_ )
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
    delete it->second;
}

inline void
nest::ConnBuilder::register_parameters_requiring_skipping_(
  ConnParameter& param )
{
  if ( param.is_array() )
  {
    parameters_requiring_skipping_.push_back( &param );
  }
}

inline void
nest::ConnBuilder::check_synapse_params_( std::string syn_name,
  const DictionaryDatum& syn_spec )
{
  // throw error if weight is specified with static_synapse_hom_w
  if ( syn_name == "static_synapse_hom_w" )
  {
    if ( syn_spec->known( names::weight ) )
      throw BadProperty(
        "Weight cannot be specified since it needs to be equal "
        "for all connections when static_synapse_hom_w is used." );
    return;
  }


  // throw error if n or a are set in quantal_stp_synapse, Connect cannot handle
  // them since they are integer
  if ( syn_name == "quantal_stp_synapse" )
  {
    if ( syn_spec->known( names::n ) )
      throw NotImplemented(
        "Connect doesn't support the setting of parameter "
        "n in quantal_stp_synapse. Use SetDefaults() or CopyModel()." );
    if ( syn_spec->known( names::a ) )
      throw NotImplemented(
        "Connect doesn't support the setting of parameter "
        "a in quantal_stp_synapse. Use SetDefaults() or CopyModel()." );
    return;
  }

  // print warning if delay is specified outside cont_delay_synapse
  if ( syn_name == "cont_delay_synapse" )
  {
    if ( syn_spec->known( names::delay ) )
      LOG( M_WARNING,
        "Connect",
        "The delay will be rounded to the next multiple of the time step. "
        "To use a more precise time delay it needs to be defined within "
        "the synapse, e.g. with CopyModel()." );
    return;
  }

  // throw error if no volume transmitter is defined or parameters are specified
  // that need to be introduced via CopyModel or SetDefaults
  if ( syn_name == "stdp_dopamine_synapse" )
  {
    if ( syn_spec->known( "vt" ) )
      throw NotImplemented(
        "Connect doesn't support the direct specification of the "
        "volume transmitter of stdp_dopamine_synapse in syn_spec."
        "Use SetDefaults() or CopyModel()." );
    // setting of parameter c and n not thread save
    if ( kernel().vp_manager.get_num_threads() > 1 )
    {
      if ( syn_spec->known( names::c ) )
        throw NotImplemented(
          "For multi-threading Connect doesn't support the setting "
          "of parameter c in stdp_dopamine_synapse. "
          "Use SetDefaults() or CopyModel()." );
      if ( syn_spec->known( names::n ) )
        throw NotImplemented(
          "For multi-threading Connect doesn't support the setting "
          "of parameter n in stdp_dopamine_synapse. "
          "Use SetDefaults() or CopyModel()." );
    }
    std::string param_arr[] = {
      "A_minus", "A_plus", "Wmax", "Wmin", "b", "tau_c", "tau_n", "tau_plus"
    };
    std::vector< std::string > param_vec( param_arr, param_arr + 8 );
    for ( std::vector< std::string >::iterator it = param_vec.begin();
          it != param_vec.end();
          it++ )
    {
      if ( syn_spec->known( *it ) )
        throw NotImplemented(
          "Connect doesn't support the setting of parameter " + *it
          + " in stdp_dopamine_synapse. Use SetDefaults() or CopyModel()." );
    }
    return;
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
int
nest::ConnBuilder::change_connected_synaptic_elements( index sgid,
  index tgid,
  const int tid,
  int update )
{

  int local = 1;
  // check whether the source is on this mpi machine
  if ( kernel().node_manager.is_local_gid( sgid ) )
  {
    Node* const source = kernel().node_manager.get_node( sgid );
    const thread source_thread = source->get_thread();

    // check whether the source is on our thread
    if ( tid == source_thread )
    {
      // update the number of connected synaptic elements
      source->connect_synaptic_element( pre_synaptic_element_name, update );
    }
  }

  // check whether the target is on this mpi machine
  if ( not kernel().node_manager.is_local_gid( tgid ) )
  {
    local = 0;
  }
  else
  {
    Node* const target = kernel().node_manager.get_node( tgid );
    const thread target_thread = target->get_thread();
    // check whether the target is on our thread
    if ( tid != target_thread )
      local = 0;
    else
    {
      // update the number of connected synaptic elements
      target->connect_synaptic_element( post_synaptic_element_name, update );
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
  if ( symmetric_ && not supports_symmetric() )
    throw NotImplemented(
      "This connection rule does not support symmetric connections." );

  if ( pre_synaptic_element_name != "" && post_synaptic_element_name != "" )
  {
    if ( symmetric_ )
      throw NotImplemented(
        "Symmetric connections are not supported in combination with "
        "structural plasticity." );
    sp_connect_();
  }
  else
  {
    connect_();
    if ( symmetric_ )
    {
      // call reset on all parameters
      if ( weight_ )
        weight_->reset();
      if ( delay_ )
        delay_->reset();
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
  for ( size_t thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
    if ( exceptions_raised_.at( thr ).valid() )
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
}

/**
 * Now we can delete synapses with or without structural plasticity
 */
void
nest::ConnBuilder::disconnect()
{
  if ( pre_synaptic_element_name != "" && post_synaptic_element_name != "" )
  {
    sp_disconnect_();
  }
  else
  {
    disconnect_();
  }

  // check if any exceptions have been raised
  for ( index thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
    if ( exceptions_raised_.at( thr ).valid() )
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
}

inline void
nest::ConnBuilder::single_connect_( index sgid,
  Node& target,
  thread target_thread,
  librandom::RngPtr& rng )
{
  if ( param_dicts_.empty() ) // indicates we have no synapse params
  {
    if ( default_weight_and_delay_ )
      kernel().connection_manager.connect(
        sgid, &target, target_thread, synapse_model_ );
    else if ( default_weight_ )
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_,
        delay_->value_double( target_thread, rng ) );
    else
    {
      double delay = delay_->value_double( target_thread, rng );
      double weight = weight_->value_double( target_thread, rng );
      kernel().connection_manager.connect(
        sgid, &target, target_thread, synapse_model_, delay, weight );
    }
  }
  else
  {
    assert( kernel().vp_manager.get_num_threads() == param_dicts_.size() );

    for ( ConnParameterMap::const_iterator it = synapse_params_.begin();
          it != synapse_params_.end();
          ++it )
    {
      if ( it->first == names::receptor_type
        || it->first == names::music_channel
        || it->first == names::synapse_label )
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
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_,
        param_dicts_[ target_thread ] );
    else if ( default_weight_ )
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_,
        param_dicts_[ target_thread ],
        delay_->value_double( target_thread, rng ) );
    else
    {
      double delay = delay_->value_double( target_thread, rng );
      double weight = weight_->value_double( target_thread, rng );
      kernel().connection_manager.connect( sgid,
        &target,
        target_thread,
        synapse_model_,
        param_dicts_[ target_thread ],
        delay,
        weight );
    }
  }
}

inline void
nest::ConnBuilder::skip_conn_parameter_( thread target_thread )
{
  for ( std::vector< ConnParameter* >::iterator it =
          parameters_requiring_skipping_.begin();
        it != parameters_requiring_skipping_.end();
        ++it )
    ( *it )->skip( target_thread );
}

inline void
nest::ConnBuilder::single_disconnect_( index sgid,
  Node& target,
  thread target_thread )
{
  // index tgid = target.get_gid();
  // This is the most simple case in which only the synapse_model_ has been
  // defined. TODO: Add functionality to delete synapses with a given weight
  // or a given delay
  kernel().sp_manager.disconnect(
    sgid, &target, target_thread, synapse_model_ );
}

void
nest::ConnBuilder::set_pre_synaptic_element_name( std::string name )
{
  pre_synaptic_element_name = name;
}

void
nest::ConnBuilder::set_post_synaptic_element_name( std::string name )
{
  post_synaptic_element_name = name;
}

void
nest::OneToOneBuilder::connect_()
{
  // make sure that target and source population have the same size
  if ( sources_->size() != targets_->size() )
  {
    LOG( M_ERROR,
      "Connect",
      "Source and Target population must be of the same size." );
    throw DimensionMismatch();
  }

#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

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
          continue;

        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
        {
          skip_conn_parameter_( tid );
          continue;
        }

        Node* const target = kernel().node_manager.get_node( *tgid );
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
  // make sure that target and source population have the same size
  if ( sources_->size() != targets_->size() )
  {
    LOG( M_ERROR,
      "Disconnect",
      "Source and Target population must be of the same size." );
    throw DimensionMismatch();
  }

#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

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
          skip_conn_parameter_( tid );
          continue;
        }

        Node* const target = kernel().node_manager.get_node( *tgid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
        {
          skip_conn_parameter_( tid );
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
  // make sure that target and source population have the same size
  if ( sources_->size() != targets_->size() )
  {
    LOG( M_ERROR,
      "Connect",
      "Source and Target population must be of the same size." );
    throw DimensionMismatch();
  }

#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

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
          continue;

        if ( !change_connected_synaptic_elements( *sgid, *tgid, tid, 1 ) )
        {
          skip_conn_parameter_( tid );
          continue;
        }
        Node* const target = kernel().node_manager.get_node( *tgid );
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

/**
 * Solves the disconnection of two nodes on a OneToOne basis with
 * structural plasticity. This means this method is used by the
 * structural plasticity manager based on the homostatic rules defined
 * for the synaptic elements on each node.
 */
void
nest::OneToOneBuilder::sp_disconnect_()
{
  // make sure that target and source population have the same size
  if ( sources_->size() != targets_->size() )
  {
    LOG( M_ERROR,
      "Disconnect",
      "Source and Target population must be of the same size." );
    throw DimensionMismatch();
  }

#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

    try
    {
      for ( GIDCollection::const_iterator tgid = targets_->begin(),
                                          sgid = sources_->begin();
            tgid != targets_->end();
            ++tgid, ++sgid )
      {
        assert( sgid != sources_->end() );

        if ( !change_connected_synaptic_elements( *sgid, *tgid, tid, -1 ) )
          continue;
        Node* const target = kernel().node_manager.get_node( *tgid );
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
    const int tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
        {
          for ( GIDCollection::const_iterator sgid = sources_->begin();
                sgid != sources_->end();
                ++sgid )
            skip_conn_parameter_( tid );
          continue;
        }

        Node* const target = kernel().node_manager.get_node( *tgid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
        {
          for ( GIDCollection::const_iterator sgid = sources_->begin();
                sgid != sources_->end();
                ++sgid )
            skip_conn_parameter_( tid );
          continue;
        }

        for ( GIDCollection::const_iterator sgid = sources_->begin();
              sgid != sources_->end();
              ++sgid )
        {
          if ( not autapses_ and *sgid == *tgid )
          {
            skip_conn_parameter_( target_thread );
            continue;
          }

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
    const int tid = kernel().vp_manager.get_thread_id();
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
          if ( !change_connected_synaptic_elements( *sgid, *tgid, tid, 1 ) )
          {
            for ( GIDCollection::const_iterator sgid = sources_->begin();
                  sgid != sources_->end();
                  ++sgid )
              skip_conn_parameter_( tid );
            continue;
          }
          Node* const target = kernel().node_manager.get_node( *tgid );
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
    const int tid = kernel().vp_manager.get_thread_id();

    try
    {
      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
        {
          for ( GIDCollection::const_iterator sgid = sources_->begin();
                sgid != sources_->end();
                ++sgid )
            skip_conn_parameter_( tid );
          continue;
        }

        Node* const target = kernel().node_manager.get_node( *tgid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
        {
          for ( GIDCollection::const_iterator sgid = sources_->begin();
                sgid != sources_->end();
                ++sgid )
            skip_conn_parameter_( tid );
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
    const int tid = kernel().vp_manager.get_thread_id();

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
          if ( !change_connected_synaptic_elements( *sgid, *tgid, tid, -1 ) )
          {
            for ( GIDCollection::const_iterator sgid = sources_->begin();
                  sgid != sources_->end();
                  ++sgid )
              skip_conn_parameter_( tid );
            continue;
          }
          Node* const target = kernel().node_manager.get_node( *tgid );
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
  , indegree_( ( *conn_spec )[ Name( "indegree" ) ] )
{
  // check for potential errors

  // verify that indegree is not larger than source population if multapses are
  // disabled
  if ( not multapses_ )
  {
    if ( ( indegree_ > static_cast< long >( sources_->size() ) ) )
      throw BadProperty( "Indegree cannot be larger than population size." );
  }
}

void
nest::FixedInDegreeBuilder::connect_()
{
#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
          continue;

        Node* const target = kernel().node_manager.get_node( *tgid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
          continue;

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
          } while ( ( not autapses_ and sgid == *tgid )
            || ( not multapses_ and ch_ids.find( s_id ) != ch_ids.end() ) );

          if ( not multapses_ )
            ch_ids.insert( s_id );

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

nest::FixedOutDegreeBuilder::FixedOutDegreeBuilder(
  const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
  : ConnBuilder( sources, targets, conn_spec, syn_spec )
  , outdegree_( ( *conn_spec )[ Name( "outdegree" ) ] )
{
  // check for potential errors

  // verify that outdegree is not larger than target population if multapses are
  // disabled
  if ( not multapses_ )
  {
    if ( ( outdegree_ > static_cast< long >( targets_->size() ) ) )
      throw BadProperty( "Outdegree cannot be larger than population size." );
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
        || ( not multapses_ and ch_ids.find( t_id ) != ch_ids.end() ) );

      if ( not multapses_ )
        ch_ids.insert( t_id );

      tgt_ids_.push_back( tgid );
    }

#pragma omp parallel
    {
      // get thread id
      const int tid = kernel().vp_manager.get_thread_id();

      try
      {
        // allocate pointer to thread specific random generator
        librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

        for ( std::vector< index >::const_iterator tgid = tgt_ids_.begin();
              tgid != tgt_ids_.end();
              ++tgid )
        {
          // check whether the target is on this mpi machine
          if ( not kernel().node_manager.is_local_gid( *tgid ) )
            continue;

          Node* const target = kernel().node_manager.get_node( *tgid );
          const thread target_thread = target->get_thread();

          // check whether the target is on our thread
          if ( tid != target_thread )
            continue;

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
  , N_( ( *conn_spec )[ Name( "N" ) ] )
{

  // check for potential errors

  // verify that total number of connections is not larger than
  // N_sources*N_targets
  if ( not multapses_ )
  {
    if ( ( N_ > static_cast< long >( sources_->size() * targets_->size() ) ) )
      throw BadProperty(
        "Total number of connections cannot exceed product "
        "of source and targer population sizes." );
  }

  // for now multapses cannot be forbidden
  // TODO: Implement option for multapses_ = False, where already existing
  // connections are stored in
  // a bitmap
  if ( not multapses_ )
    throw NotImplemented(
      "Connect doesn't support the suppression of multapses in the "
      "FixedTotalNumber connector." );
}

void
nest::FixedTotalNumberBuilder::connect_()
{
  const int_t M = kernel().vp_manager.get_num_virtual_processes();
  const long_t size_sources = sources_->size();
  const long_t size_targets = targets_->size();

  // drawing connection ids

  // Compute the distribution of targets over processes using the modulo
  // function
  std::vector< std::vector< size_t > > targets_on_vp( M );
  for ( size_t t = 0; t < targets_->size(); t++ )
  {
    targets_on_vp[ kernel().vp_manager.suggest_vp( ( *targets_ )[ t ] ) ]
      .push_back( ( *targets_ )[ t ] );
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
  std::vector< long_t > num_conns_on_vp( M, 0 ); // corresponds to n[]

  // calculate exact multinomial distribution
  // get global rng that is tested for synchronization for all threads
  librandom::RngPtr grng = kernel().rng_manager.get_grng();

  // HEP: instead of counting upwards, we might count remaining_targets and
  // remaining_partitions down. why?
  // begin code adapted from gsl 1.8 //
  double_t sum_dist = 0.0; // corresponds to sum_p
  // norm is equivalent to size_targets
  uint_t sum_partitions = 0; // corresponds to sum_n
// substituting gsl_ran call
#ifdef HAVE_GSL
  librandom::GSL_BinomialRandomDev bino( grng, 0, 0 );
#else
  librandom::BinomialRandomDev bino( grng, 0, 0 );
#endif

  for ( int k = 0; k < M; k++ )
  {
    if ( targets_on_vp[ k ].size() > 0 )
    {
      double_t num_local_targets =
        static_cast< double_t >( targets_on_vp[ k ].size() );
      double_t p_local = num_local_targets / ( size_targets - sum_dist );
      bino.set_p( p_local );
      bino.set_n( N_ - sum_partitions );
      num_conns_on_vp[ k ] = bino.ldev();
    }

    sum_dist += static_cast< double_t >( targets_on_vp[ k ].size() );
    sum_partitions += static_cast< uint_t >( num_conns_on_vp[ k ] );
  }

// end code adapted from gsl 1.8

#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      const int_t vp_id = kernel().vp_manager.thread_to_vp( tid );

      if ( kernel().vp_manager.is_local_vp( vp_id ) )
      {
        librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

        while ( num_conns_on_vp[ vp_id ] > 0 )
        {

          // draw random numbers for source node from all source neurons
          const long_t s_index = rng->ulrand( size_sources );
          // draw random numbers for target node from
          // targets_on_vp on this virtual process
          const long_t t_index = rng->ulrand( targets_on_vp[ vp_id ].size() );
          // map random number of source node to gid corresponding to
          // the source_adr vector
          const long_t sgid = ( *sources_ )[ s_index ];
          // map random number of target node to gid using the
          // targets_on_vp vector
          const long_t tgid = targets_on_vp[ vp_id ][ t_index ];

          Node* const target = kernel().node_manager.get_node( tgid );
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
  , p_( ( *conn_spec )[ Name( "p" ) ] )
{
}


void
nest::BernoulliBuilder::connect_()
{
#pragma omp parallel
  {
    // get thread id
    const int tid = kernel().vp_manager.get_thread_id();

    try
    {
      // allocate pointer to thread specific random generator
      librandom::RngPtr rng = kernel().rng_manager.get_rng( tid );

      for ( GIDCollection::const_iterator tgid = targets_->begin();
            tgid != targets_->end();
            ++tgid )
      {
        // check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_gid( *tgid ) )
          continue;

        Node* const target = kernel().node_manager.get_node( *tgid );
        const thread target_thread = target->get_thread();

        // check whether the target is on our thread
        if ( tid != target_thread )
          continue;

        for ( GIDCollection::const_iterator sgid = sources_->begin();
              sgid != sources_->end();
              ++sgid )
        {
          // not possible to create multapses with this implementation,
          // hence leave out the check for BernoulliBuilder

          if ( not autapses_ and *sgid == *tgid )
            continue;

          if ( not( rng->drand() < p_ ) )
            continue;

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
  if ( pre_synaptic_element_name == "" || post_synaptic_element_name == "" )
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
    d = Time( Time::ms( getValue< double_t >( syn_defaults, "delay" ) ) )
          .get_steps();
  }
}

void
nest::SPBuilder::sp_connect( GIDCollection sources, GIDCollection targets )
{
  connect_( sources, targets );

  // check if any exceptions have been raised
  for ( size_t thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
    if ( exceptions_raised_.at( thr ).valid() )
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
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
    const int tid = kernel().vp_manager.get_thread_id();

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
          continue;

        if ( !change_connected_synaptic_elements( *sgid, *tgid, tid, 1 ) )
        {
          skip_conn_parameter_( tid );
          continue;
        }
        Node* const target = kernel().node_manager.get_node( *tgid );
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
