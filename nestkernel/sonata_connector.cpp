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
  // for ( auto params_vec_map : type_id_2_syn_spec_ )
  // {
  //   for ( auto params : params_vec_map.second )
  //   {
  //     for ( auto synapse_parameters : params )
  //     {
  //       delete synapse_parameters.second;
  //     }
  //   }
  // }
  type_id_2_syn_spec_.clear();
}

void
SonataConnector::connect()
{
  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );

  for ( auto edge_dictionary_datum : edges )
  {
    const auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    const auto edge_file = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

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
    H5::H5File file;
    try
    {
      // TODO: H5File has user-provided copy-constructor, so using the implicitly-declared operator=() generates a
      // warning.
      file = H5::H5File( edge_file, H5F_ACC_RDONLY );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Could not open HDF5 file " + edge_file );
    }

    H5::Group edges_group( file.openGroup( "edges" ) );

    // Get name of groups
    std::vector< std::string > edge_group_names;
    // https://support.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-Iterate
    H5Literate( edges_group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_group_names, &edge_group_names );

    // Iterates the groups of "edges"
    for ( const auto& group_name : edge_group_names )
    {
      std::cerr << "Group: " << group_name << "\n";
      const H5::Group edges_subgroup( edges_group.openGroup( group_name ) );

      // Open edge_group_id dataset and check if we have more than one group id. Currently only one is allowed
      std::cerr << "Open dataset edge_group_id...\n";
      const auto edge_group_id = edges_subgroup.openDataSet( "edge_group_id" );
      const auto num_edge_group_id = get_num_elements_( edge_group_id );
      const auto edge_group_id_data = read_data_( edge_group_id, num_edge_group_id );
      const auto [ min, max ] = std::minmax_element( edge_group_id_data, edge_group_id_data + num_edge_group_id );

      if ( *min == *max ) // only one group_id
      {
        const auto edge_parameters = edges_subgroup.openGroup( std::to_string( *min ) );

        // Read source and target dataset
        std::cerr << "Open dataset source_node_id...\n";
        const auto source_node_id = edges_subgroup.openDataSet( "source_node_id" );
        const auto num_source_node_id = get_num_elements_( source_node_id );
        const auto source_node_id_data = read_data_( source_node_id, num_source_node_id );

        std::cerr << "Open dataset target_node_id...\n";
        const auto target_node_id = edges_subgroup.openDataSet( "target_node_id" );
        const auto num_target_node_id = get_num_elements_( target_node_id );
        const auto target_node_id_data = read_data_( target_node_id, num_target_node_id );

        // Check if weight and delay are given as h5 files, if so, read the datasets
        weight_and_delay_from_dataset_( edge_parameters );
        auto read_dataset = [&edge_parameters]( double*& data, const char* data_set_name ) {
          std::cerr << "(lambda) read " << data_set_name << " dataset...\n";
          const auto dataset = edge_parameters.openDataSet( data_set_name );
          std::cerr << "(lambda) StorageSize = " << dataset.getStorageSize() << "\n";
          data = new double[ dataset.getStorageSize() ];
          dataset.read( data, dataset.getDataType() );
          return data;
        };

        if ( weight_dataset_ )
        {
          read_dataset( syn_weight_data_, "syn_weight" );
        }
        if ( delay_dataset_ )
        {
          read_dataset( delay_data_, "delay" );
        }

        std::cerr << "get_data_...\n";
        // Get edge_type_id, these are later mapped to the different synapse parameters
        const auto edge_type_id_data = get_data_( edges_subgroup, "edge_type_id" );

        std::cerr << "get_attributes_ source node_population...\n";
        // Retrieve source and target attributes to find which node population to map to
        std::string source_attribute_value;
        get_attributes_( source_attribute_value, source_node_id, "node_population" );

        std::cerr << "get_attributes_ target node_population...\n";
        std::string target_attribute_value;
        get_attributes_( target_attribute_value, target_node_id, "node_population" );

        // Create map of edge type ids to NEST synapse_model ids
        const auto edge_params = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );
        std::cerr << "create_type_id_2_syn_spec_...\n";
        create_type_id_2_syn_spec_( edge_params );

        assert( num_source_node_id == num_target_node_id );

        std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_(
          kernel().vp_manager.get_num_threads() );

        std::cerr << "Enter parallel region...\n";

#pragma omp parallel
        {
          const auto tid = kernel().vp_manager.get_thread_id();

          try
          {
            // Retrieve the correct NodeCollections
            const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
            const auto current_source_nc =
              getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value ) );
            const auto current_target_nc =
              getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value ) );

            auto snode_it = current_source_nc->begin();
            auto tnode_it = current_target_nc->begin();


            // Iterate the datasets and create the connections
            for ( hsize_t i = 0; i < num_source_node_id; ++i )
            {
              const auto sonata_source_id = source_node_id_data[ i ];
              const index snode_id = ( *( snode_it + sonata_source_id ) ).node_id;

              const auto sonata_target_id = target_node_id_data[ i ];
              const index target_id = ( *( tnode_it + sonata_target_id ) ).node_id;
              Node* target = kernel().node_manager.get_node_or_proxy( target_id );

              if ( not target ) // TODO: remove
              {
#pragma omp critical
                {
                  std::cerr << kernel().vp_manager.get_thread_id() << ": " << target_id << " node_or_proxy is NULL!\n";
                }
              }

              const thread target_thread = target->get_thread();

              // Skip if target is not on this thread, or not on this MPI process.
              if ( target->is_proxy() or tid != target_thread )
              {
                continue;
              }

              const auto edge_type_id = edge_type_id_data[ i ];
              const auto syn_spec =
                getValue< DictionaryDatum >( edge_params->lookup( std::to_string( edge_type_id ) ) );

              auto get_syn_property = [&syn_spec, &i]( const bool dataset, const double* data, const Name& name ) {
                if ( dataset ) // Syn_property is set from dataset if the dataset is defined
                {
                  if ( not data[ i ] ) // TODO: remove
                  {
#pragma omp critical
                    {
                      std::cerr << kernel().vp_manager.get_thread_id() << ": " << name << " " << i
                                << " data index is NULL!\n";
                    }
                  }
                  return data[ i ];
                }
                else if ( syn_spec->known( name ) ) // Set syn_property from syn_spec if it is defined there
                {
                  return static_cast< double >( ( *syn_spec )[ name ] );
                }
                return numerics::nan; // Default value is NaN
              };

              const double weight = get_syn_property( weight_dataset_, syn_weight_data_, names::weight );
              const double delay = get_syn_property( delay_dataset_, delay_data_, names::delay );

              RngPtr rng = get_vp_specific_rng( target_thread );
              get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

              kernel().connection_manager.connect( snode_id,
                target,
                target_thread,
                type_id_2_syn_model_.at( edge_type_id ),
                type_id_2_param_dicts_.at( edge_type_id ).at( tid ),
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
SonataConnector::get_num_elements_( const H5::DataSet& dataset )
{
  std::cerr << "get_num_elements_...\n";
  auto dataspace = dataset.getSpace();
  hsize_t dims_out[ 1 ];
  dataspace.getSimpleExtentDims( dims_out, NULL );
  std::cerr << "dims_out: " << *dims_out << "\n";
  return *dims_out;
}

int*
SonataConnector::read_data_( const H5::DataSet& dataset, hsize_t num_elements )
{
  std::cerr << kernel().vp_manager.get_thread_id() << " read_data_...\n";
  std::cerr << "Num_elements: " << num_elements << "\n";

  int* data = ( int* ) malloc( num_elements * sizeof( int ) );
#pragma omp critical // TODO: revert
  {
    try
    {
      dataset.read( data, H5::PredType::NATIVE_INT );
    }
    catch ( std::exception const& e )
    {
      std::cerr << __FILE__ << ":" << __LINE__ << " : "
                << "Exception caught " << e.what() << "\n";
    }
    catch ( const H5::Exception& e )
    {
      // std::cerr << __FILE__ << ":" << __LINE__ << " : " << "H5 exception caught:\n" << e.what() << "\n";
      std::cerr << __FILE__ << ":" << __LINE__ << " H5 exception caught: " << e.getDetailMsg() << "\n";
      e.printErrorStack();
    }
    catch ( ... )
    {
      std::cerr << __FILE__ << ":" << __LINE__ << " : "
                << "Unknown exception caught\n";
    }
  }
  return data;
}

int*
SonataConnector::get_data_( const H5::Group& group, const std::string& name )
{
  auto dataset = group.openDataSet( name );
  auto num_data = get_num_elements_( dataset );
  return read_data_( dataset, num_data );
}

void
SonataConnector::get_attributes_( std::string& attribute_value,
  const H5::DataSet& dataset,
  const std::string& attribute_name )
{
  H5::Attribute attr = dataset.openAttribute( attribute_name );
  H5::DataType type = attr.getDataType();
  attr.read( type, attribute_value );
}

void
SonataConnector::weight_and_delay_from_dataset_( const H5::Group& group )
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
    std::cerr << "type_id=" << type_id << " syn_name=" << syn_name << "\n";

    // The following call will throw "UnknownSynapseType" if syn_name is not naming a known model
    const index synapse_model_id = kernel().model_manager.get_synapse_model_id( syn_name );
    std::cerr << "synapse_model_id=" << synapse_model_id << "\n";

    set_synapse_params( d, synapse_model_id, type_id );
    std::cerr << "synapse_model_id=" << synapse_model_id << "\n";
    type_id_2_syn_model_[ type_id ] = synapse_model_id;
  }
}

void
SonataConnector::set_synapse_params( DictionaryDatum syn_dict, index synapse_model_id, int type_id )
{
  std::cerr << "set_synapse_params...\n";
  DictionaryDatum syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_id );
  std::set< Name > skip_syn_params_ = {
    names::weight, names::delay, names::min_delay, names::max_delay, names::num_connections, names::synapse_model
  };

  std::map< Name, std::shared_ptr< ConnParameter > > synapse_params; // TODO: Use unique_ptr/shared_ptr

  for ( Dictionary::const_iterator default_it = syn_defaults->begin(); default_it != syn_defaults->end(); ++default_it )
  {
    const Name param_name = default_it->first;
    if ( skip_syn_params_.find( param_name ) != skip_syn_params_.end() )
    {
      continue; // weight, delay or other not-settable parameter
    }

    if ( syn_dict->known( param_name ) )
    {
      synapse_params[ param_name ] = std::shared_ptr< ConnParameter >(
        ConnParameter::create( ( *syn_dict )[ param_name ], kernel().vp_manager.get_num_threads() ) );
    }
  }

  // Now create dictionary with dummy values that we will use to pass settings to the synapses created. We
  // create it here once to avoid re-creating the object over and over again.
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    std::cerr << "type_id_2_syn_spec_.at...\n";
    type_id_2_syn_spec_[ type_id ].push_back( synapse_params ); // DO WE NEED TO DEFINE THIS PER THREAD???
    std::cerr << "type_id_2_param_dicts_.at...\n";
    type_id_2_param_dicts_[ type_id ].push_back( new Dictionary );

    for ( auto param : synapse_params )
    {
      if ( param.second->provides_long() )
      {
        std::cerr << "int type_id_2_param_dicts_.at...\n";
        ( *type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new IntegerDatum( 0 ) );
      }
      else
      {
        std::cerr << "double type_id_2_param_dicts_.at...\n";
        ( *type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new DoubleDatum( 0.0 ) );
      }
    }
  }
}

void
SonataConnector::get_synapse_params_( index snode_id, Node& target, thread target_thread, RngPtr rng, int edge_type_id )
{
  for ( auto const& syn_param : type_id_2_syn_spec_.at( edge_type_id ).at( target_thread ) )
  {
    const Name param_name = syn_param.first;
    const auto param = syn_param.second;

    if ( param->provides_long() )
    {
      // change value of dictionary entry without allocating new datum
      IntegerDatum* dd = static_cast< IntegerDatum* >(
        ( ( *type_id_2_param_dicts_.at( edge_type_id ).at( target_thread ) )[ param_name ] ).datum() );
      ( *dd ) = param->value_int( target_thread, rng, snode_id, &target );
    }
    else
    {
      // change value of dictionary entry without allocating new datum
      DoubleDatum* dd = static_cast< DoubleDatum* >(
        ( ( *type_id_2_param_dicts_.at( edge_type_id ).at( target_thread ) )[ param_name ] ).datum() );
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
