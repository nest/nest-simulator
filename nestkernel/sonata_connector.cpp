/*
 *  conn_builder_sonata.cpp
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


// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

#include <iostream>
#include <fstream>

#include "H5Cpp.h"  // TODO: need an if/else

extern "C" herr_t get_group_names( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

SonataConnector::SonataConnector( const DictionaryDatum& sonata_dynamics )
  : sonata_dynamics_ ( sonata_dynamics )
  , weight_dataset_ ( false )
  , delay_dataset_ ( false )
  , syn_weight_data_ ( 0 )
  , delay_data_ ( 0 )
  , param_dict_ ( new Dictionary() )
{
}

void
SonataConnector::connect()
{
  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );

  std::ofstream MyFile("check_conns.txt");

  for ( auto edge_dictionary_datum : edges )
  {
    auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    auto edge_file = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    // Open the specified file and the specified group in the file.
    H5::H5File file( edge_file, H5F_ACC_RDONLY );
    H5::Group edges_group( file.openGroup( "edges" ) );

    // Get name of groups
    std::vector< std::string > edge_group_names;
    // https://support.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-Iterate
    H5Literate( edges_group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_group_names, &edge_group_names );

    for ( auto&& group_name : edge_group_names )
    {
      H5::Group edges_subgroup( edges_group.openGroup( group_name ) );

      auto edge_group_id = edges_subgroup.openDataSet( "edge_group_id" );
      auto num_edge_group_id = get_num_elements_( edge_group_id );
      auto edge_group_id_data = read_data_( edge_group_id, num_edge_group_id );
      const auto [min, max] = std::minmax_element(edge_group_id_data, edge_group_id_data + num_edge_group_id);

      if ( *min == *max ) // only one group_id
      {
        auto edge_parameters = edges_subgroup.openGroup( std::to_string( *min ) );

        auto source_node_id = edges_subgroup.openDataSet( "source_node_id" );
        auto num_source_node_id = get_num_elements_( source_node_id );
        auto source_node_id_data = read_data_( source_node_id, num_source_node_id );

        auto target_node_id = edges_subgroup.openDataSet( "target_node_id" );
        auto num_target_node_id = get_num_elements_( target_node_id );
        auto target_node_id_data = read_data_( target_node_id, num_target_node_id );

        // Check if weight and delay are given as h5 files
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

        auto edge_type_id_data = get_data_( edges_subgroup, "edge_type_id" ); //synapses

        // Retrieve source and target attributes to find which node population to map to
        std::string source_attribute_value;
        get_attributes_( source_attribute_value, source_node_id, "node_population" );

        std::string target_attribute_value;
        get_attributes_( target_attribute_value, target_node_id, "node_population" );

        // Create map of edge type ids to NEST synapse_model ids
        create_type_id_2_syn_spec_( edge_dict );

        assert( num_source_node_id == num_target_node_id );

        // Retrieve parameters
        auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
        auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value ) );
        auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value ) );

        // Connect
        auto snode_it = current_source_nc->begin();
        auto tnode_it = current_target_nc->begin();
        for ( hsize_t i = 0; i < num_source_node_id; ++i )  // iterate sonata files
        {
          const auto sonata_source_id = source_node_id_data[ i ];
          const index snode_id = ( *( snode_it + sonata_source_id ) ).node_id;

          const auto sonata_target_id = target_node_id_data[ i ];
          const index target_id = ( *( tnode_it + sonata_target_id ) ).node_id;
          Node* target = kernel().node_manager.get_node_or_proxy( target_id );

          thread target_thread = target->get_thread();

          const auto syn_spec = type_id_2_syn_spec_[ edge_type_id_data[ i ] ];
          const auto model_name = getValue< std::string >( ( *syn_spec )[ "synapse_model" ] );
          index synapse_model_id = kernel().model_manager.get_synapsedict()->lookup( model_name );

          double weight = numerics::nan;
          if ( weight_dataset_ )
          {
            weight = syn_weight_data_[ i ];
          }
          else if ( syn_spec->known( names::weight ) )
          {
            weight = std::stod( ( *syn_spec )[ names::weight ] );
          }

          double delay = numerics::nan;
          if ( delay_dataset_ )
          {
            delay = syn_weight_data_[ i ];
          }
          else if ( syn_spec->known( names::delay ) )
          {
            delay = std::stod( ( *syn_spec )[ names::delay ] );
          }

          RngPtr rng = get_vp_specific_rng( target_thread );
          get_synapse_params_( syn_spec, snode_id, *target, target_thread, rng );

          //if ( i % 100000 == 0 )
          //{
            //std::cerr << "connection number " << i << "\n";
          //std::cerr << "connection number " << i << " source " << snode_id << " target " << target_id << " weight " << weight << " delay " << delay << "\n";

          MyFile << "connection number " << i << " source " << snode_id << " target " << target_id << " weight " << weight << " delay " << delay << "\n";
            //std::cerr << "source node id data " << sonata_source_id << "\n";
            //std::cerr << "snode_it begin " << (*snode_it).node_id << "\n";
          //}


          kernel().connection_manager.connect( snode_id,
            target,
            target_thread,
            synapse_model_id,
            param_dict_,
            delay,
            weight );
        }

        delete source_node_id_data;
        delete target_node_id_data;
        if ( weight_dataset_ )
        {
          delete syn_weight_data_;
        }
        if ( delay_dataset_ )
        {
          delete delay_data_;
        }
        delete edge_type_id_data;
      }
      else
      {
        throw NotImplemented( "Connecting with Sonata files with more than one edgegroup is currently not implemented" );
      }
      /*
       * Close the dataset.
       */
      delete edge_group_id_data;
    }
    edges_group.close();
    file.close();
  }
  MyFile.close();
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
SonataConnector::create_type_id_2_syn_spec_( DictionaryDatum edge_dict )
{
  auto edge_params = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );

  for ( auto it = edge_params->begin(); it != edge_params->end(); ++it )
  {
    const auto type_id = std::stoi( it->first.toString() );
    auto d = getValue< DictionaryDatum >( it->second );
    const auto model_name = getValue< std::string >( ( *d )[ "synapse_model" ] );

    if ( not kernel().model_manager.get_synapsedict()->known( model_name ) )
    {
      throw UnknownSynapseType( model_name );
    }
    type_id_2_syn_spec_[ type_id ] = d;
  }
}

void
SonataConnector::get_synapse_params_( DictionaryDatum syn_params, index snode_id, Node& target, thread target_thread, RngPtr rng )
{
  std::set< Name > skip_syn_params_ = { names::weight, names::delay, names::synapse_model };

  for ( auto syn_param_it = syn_params->begin(); syn_param_it != syn_params->end(); ++syn_param_it )
  {
    const Name param_name = syn_param_it->first;
    if ( skip_syn_params_.find( param_name ) != skip_syn_params_.end() )
    {
      continue; // weight, delay or other non-settable parameter
    }

    auto parameter = ConnParameter::create( ( *syn_params )[ param_name ], kernel().vp_manager.get_num_threads() );

    if ( parameter->provides_long() )
    {
      ( *param_dict_ )[ param_name ] = parameter->value_int( target_thread, rng, snode_id, &target );
    }
    else
    {
      ( *param_dict_ )[ param_name ] = parameter->value_double( target_thread, rng, snode_id, &target );
    }
  }
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

