/*
 *  sonata_connector.cpp
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

#include "sonata_connector.h"

#include "config.h"

#ifdef HAVE_HDF5

// Includes from nestkernel:
#include "conn_parameter.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

#include <fstream>  // for debugging
#include <iostream> // for debugging

#include "H5Cpp.h"

extern "C" herr_t get_group_names( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

SonataConnector::SonataConnector( const DictionaryDatum& sonata_dynamics )
  : sonata_dynamics_( sonata_dynamics )
  , weight_dataset_( false )
  , delay_dataset_( false )
  , syn_weight_data_( 0 )
  , delay_data_( 0 )
{
}

SonataConnector::~SonataConnector()
{
  for ( auto params_vec_map : type_id_2_syn_spec_ )
  {
    for ( auto params : params_vec_map.second )
    {
      for ( auto synapse_parameters : params )
      {
        delete synapse_parameters.second;
      }
    }
  }
  type_id_2_syn_spec_.clear();
}

void
SonataConnector::connect()
{
  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );

  for ( auto edge_dictionary_datum : edges )
  {
    auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    auto edge_file = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    /*
     * Structure of SONATA files:
     * edge_file.h5          (name changes)
     *   edges               (name fixed)
     *     group_name        (name changes, usually only one, can be more groups)
     *       0               (name fixed, usually only one)
     *         syn_weights   (name fixed)
     *         delays        (name fixed)
     *       edge_group_id   (name fixed)
     *       edge_type_id    (name fixed)
     *       source_node_id  (name fixed)
     *       target_node_id  (name fixed)
     */

    // Open the specified file and the specified group in the file.
    H5::H5File file( edge_file, H5F_ACC_RDONLY );
    H5::Group edges_group( file.openGroup( "edges" ) );

    // Get name of groups
    std::vector< std::string > edge_group_names;
    // https://support.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-Iterate
    H5Literate( edges_group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_group_names, &edge_group_names );

    // Iterates the groups of "edges"
    for ( auto&& group_name : edge_group_names )
    {
      H5::Group edges_subgroup( edges_group.openGroup( group_name ) );

      // Open edge_group_id dataset and check if we have more than one group id. Currently only one is allowed
      auto edge_group_id = edges_subgroup.openDataSet( "edge_group_id" );
      auto num_edge_group_id = get_num_elements_( edge_group_id );
      auto edge_group_id_data = read_data_( edge_group_id, num_edge_group_id );
      const auto [ min, max ] = std::minmax_element( edge_group_id_data, edge_group_id_data + num_edge_group_id );

      if ( *min == *max ) // only one group_id
      {
        auto edge_parameters = edges_subgroup.openGroup( std::to_string( *min ) );

        // Read source and target dataset
        auto source_node_id = edges_subgroup.openDataSet( "source_node_id" );
        auto num_source_node_id = get_num_elements_( source_node_id );
        auto source_node_id_data = read_data_( source_node_id, num_source_node_id );

        auto target_node_id = edges_subgroup.openDataSet( "target_node_id" );
        auto num_target_node_id = get_num_elements_( target_node_id );
        auto target_node_id_data = read_data_( target_node_id, num_target_node_id );

        // Check if weight and delay are given as h5 files, if so, read the datasets
        weight_and_delay_from_dataset_( edge_parameters );
        if ( weight_dataset_ )
        {
          auto weight_dataset = edge_parameters.openDataSet( "syn_weight" );
          syn_weight_data_ = new double[ weight_dataset.getStorageSize() ];
          weight_dataset.read( syn_weight_data_, weight_dataset.getDataType() );
        }
        if ( delay_dataset_ )
        {
          auto delay_dataset = edge_parameters.openDataSet( "delay" );
          delay_data_ = new double[ delay_dataset.getStorageSize() ];
          delay_dataset.read( delay_data_, delay_dataset.getDataType() );
        }

        // Get edge_type_id, these are later mapped to the different synapse parameters
        auto edge_type_id_data = get_data_( edges_subgroup, "edge_type_id" );

        // Retrieve source and target attributes to find which node population to map to
        std::string source_attribute_value;
        get_attributes_( source_attribute_value, source_node_id, "node_population" );

        std::string target_attribute_value;
        get_attributes_( target_attribute_value, target_node_id, "node_population" );

        // Create map of edge type ids to NEST synapse_model ids
        auto edge_params = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );
        create_type_id_2_syn_spec_( edge_params );

        assert( num_source_node_id == num_target_node_id );

        std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_(
          kernel().vp_manager.get_num_threads() );
#pragma omp parallel
        {
          auto tid = kernel().vp_manager.get_thread_id();

          try
          {
            // Retrieve the correct NodeCollection's
            auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
            auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value ) );
            auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value ) );

            auto snode_it = current_source_nc->begin();
            auto tnode_it = current_target_nc->begin();

            // Iterate the datasaets and create the connections
            for ( hsize_t i = 0; i < num_source_node_id; ++i )
            {
              const auto sonata_source_id = source_node_id_data[ i ];
              const index snode_id = ( *( snode_it + sonata_source_id ) ).node_id;

              const auto sonata_target_id = target_node_id_data[ i ];
              const index target_id = ( *( tnode_it + sonata_target_id ) ).node_id;
              Node* target = kernel().node_manager.get_node_or_proxy( target_id );

              thread target_thread = target->get_thread();

              if ( target->is_proxy() or tid != target_thread )
              {
                continue;
              }

              auto edge_type_id = edge_type_id_data[ i ];
              const auto syn_spec =
                getValue< DictionaryDatum >( edge_params->lookup( std::to_string( edge_type_id ) ) );

              double weight = numerics::nan;
              if ( weight_dataset_ )
              {
                weight = syn_weight_data_[ i ];
              }
              else if ( syn_spec->known( names::weight ) )
              {
                weight = ( *syn_spec )[ names::weight ];
              }

              double delay = numerics::nan;
              if ( delay_dataset_ )
              {
                delay = delay_data_[ i ];
              }
              else if ( syn_spec->known( names::delay ) )
              {
                delay = ( *syn_spec )[ names::delay ];
              }

              RngPtr rng = get_vp_specific_rng( target_thread );
              get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

              kernel().connection_manager.connect( snode_id,
                target,
                target_thread,
                type_id_2_syn_model_[ edge_type_id ],
                type_id_2_param_dicts_[ edge_type_id ][ tid ],
                delay,
                weight );
            }
          }
          catch ( std::exception& err )
          {
            // We must create a new exception here, err's lifetime ends at
            // the end of the catch block.
            exceptions_raised_.at( tid ) =
              std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
          }
        } // omp parallel
        // check if any exceptions have been raised
        for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
        {
          if ( exceptions_raised_.at( thr ).get() )
          {
            throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
          }
        }

        // Delete the datasets and reset all parameters
        delete source_node_id_data;
        delete target_node_id_data;
        delete edge_type_id_data;
        reset_params();
      }
      else
      {
        throw NotImplemented(
          "Connecting with Sonata files with more than one edgegroup is currently not implemented" );
      }
      delete edge_group_id_data;
    } // groups of "edges"

    edges_group.close();
    file.close();
  }
}

hsize_t
SonataConnector::get_num_elements_( H5::DataSet& dataset )
{
  auto dataspace = dataset.getSpace();
  hsize_t dims_out[ 1 ];
  dataspace.getSimpleExtentDims( dims_out, NULL );
  return *dims_out;
}

int*
SonataConnector::read_data_( H5::DataSet dataset, int num_elements )
{
  int* data = ( int* ) malloc( num_elements * sizeof( int ) );
  dataset.read( data, H5::PredType::NATIVE_INT );
  return data;
}

int*
SonataConnector::get_data_( H5::Group group, std::string name )
{
  auto dataset = group.openDataSet( name );
  auto num_data = get_num_elements_( dataset );
  return read_data_( dataset, num_data );
}

void
SonataConnector::get_attributes_( std::string& attribute_value, H5::DataSet dataset, std::string attribute_name )
{
  H5::Attribute attr = dataset.openAttribute( attribute_name );
  H5::DataType type = attr.getDataType();
  attr.read( type, attribute_value );
}

void
SonataConnector::weight_and_delay_from_dataset_( H5::Group group )
{
  weight_dataset_ = H5Lexists( group.getId(), "syn_weight", H5P_DEFAULT ) > 0;
  delay_dataset_ = H5Lexists( group.getId(), "delay", H5P_DEFAULT ) > 0;
}

void
SonataConnector::create_type_id_2_syn_spec_( DictionaryDatum edge_params )
{
  for ( auto it = edge_params->begin(); it != edge_params->end(); ++it )
  {
    const auto type_id = std::stoi( it->first.toString() );
    auto d = getValue< DictionaryDatum >( it->second );
    const auto syn_name = getValue< std::string >( ( *d )[ "synapse_model" ] );

    // The following call will throw "UnknownSynapseType" if syn_name is not naming a known model
    const index synapse_model_id = kernel().model_manager.get_synapse_model_id( syn_name );

    set_synapse_params( d, synapse_model_id, type_id );
    type_id_2_syn_model_[ type_id ] = synapse_model_id;
  }
}

void
SonataConnector::set_synapse_params( DictionaryDatum syn_dict, index synapse_model_id, int type_id )
{
  DictionaryDatum syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_id );
  std::set< Name > skip_syn_params_ = {
    names::weight, names::delay, names::min_delay, names::max_delay, names::num_connections, names::synapse_model
  };

  std::map< Name, ConnParameter* > synapse_params;

  for ( Dictionary::const_iterator default_it = syn_defaults->begin(); default_it != syn_defaults->end(); ++default_it )
  {
    const Name param_name = default_it->first;
    if ( skip_syn_params_.find( param_name ) != skip_syn_params_.end() )
    {
      continue; // weight, delay or other not-settable parameter
    }

    if ( syn_dict->known( param_name ) )
    {
      synapse_params[ param_name ] =
        ConnParameter::create( ( *syn_dict )[ param_name ], kernel().vp_manager.get_num_threads() );
    }
  }

  // Now create dictionary with dummy values that we will use to pass settings to the synapses created. We
  // create it here once to avoid re-creating the object over and over again.
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    type_id_2_syn_spec_[ type_id ].push_back( synapse_params ); // DO WE NEED TO DEFINE THIS PER THREAD???
    type_id_2_param_dicts_[ type_id ].push_back( new Dictionary );

    for ( auto param : synapse_params )
    {
      if ( param.second->provides_long() )
      {
        ( *type_id_2_param_dicts_[ type_id ][ tid ] )[ param.first ] = Token( new IntegerDatum( 0 ) );
      }
      else
      {
        ( *type_id_2_param_dicts_[ type_id ][ tid ] )[ param.first ] = Token( new DoubleDatum( 0.0 ) );
      }
    }
  }
}

void
SonataConnector::get_synapse_params_( index snode_id, Node& target, thread target_thread, RngPtr rng, int edge_type_id )
{
  for ( auto const& syn_param : type_id_2_syn_spec_[ edge_type_id ][ target_thread ] )
  {
    const Name param_name = syn_param.first;
    const ConnParameter* param = syn_param.second;

    if ( param->provides_long() )
    {
      // change value of dictionary entry without allocating new datum
      IntegerDatum* dd = static_cast< IntegerDatum* >(
        ( ( *type_id_2_param_dicts_[ edge_type_id ][ target_thread ] )[ param_name ] ).datum() );
      ( *dd ) = param->value_int( target_thread, rng, snode_id, &target );
    }
    else
    {
      // change value of dictionary entry without allocating new datum
      DoubleDatum* dd = static_cast< DoubleDatum* >(
        ( ( *type_id_2_param_dicts_[ edge_type_id ][ target_thread ] )[ param_name ] ).datum() );
      ( *dd ) = param->value_double( target_thread, rng, snode_id, &target );
    }
  }
}

void
SonataConnector::reset_params()
{
  if ( weight_dataset_ )
  {
    delete syn_weight_data_;
  }
  if ( delay_dataset_ )
  {
    delete delay_data_;
  }

  type_id_2_syn_model_.clear();
  for ( auto params_vec_map : type_id_2_syn_spec_ )
  {
    for ( auto params : params_vec_map.second )
    {
      for ( auto synapse_parameters : params )
      {
        synapse_parameters.second->reset();
      }
    }
  }
  type_id_2_syn_spec_.clear();
  type_id_2_param_dicts_.clear();
}

} // namespace nest

herr_t
get_group_names( hid_t loc_id, const char* name, const H5L_info_t*, void* opdata )
{
  // Check that the group exists
  herr_t status = H5Gget_objinfo( loc_id, name, 0, NULL );
  if ( status != 0 )
  {
    // Group doesn't exist, or some error occurred.
    return 0; // TODO: is this a dataset?
  }

  auto group_names = reinterpret_cast< std::vector< std::string >* >( opdata );
  group_names->push_back( name );

  return 0;
}

#endif // ifdef HAVE_HDF5
