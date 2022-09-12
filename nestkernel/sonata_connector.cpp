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

#include <cstdlib>

// Includes from nestkernel:
#include "conn_parameter.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

#include <fstream>  // for debugging
#include <iostream> // for debugging

#include "H5Cpp.h"

extern "C" herr_t get_group_names_callback( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

// constexpr hsize_t CHUNK_SIZE = 10000;
// constexpr hsize_t CHUNK_SIZE = 100000;
constexpr hsize_t CHUNK_SIZE = 1000000;
// constexpr hsize_t CHUNK_SIZE = 10000000;

SonataConnector::SonataConnector( const DictionaryDatum& sonata_dynamics )
  : sonata_dynamics_( sonata_dynamics )
  , weight_dataset_exist_( false )
  , delay_dataset_exist_( false )
{
}

SonataConnector::~SonataConnector()
{
  type_id_2_syn_spec_.clear();
}

void
SonataConnector::connect()
{

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

  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );

  for ( auto edge_dictionary_datum : edges )
  {

    const auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    const auto edge_filename = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    // Create map of edge type ids to NEST synapse_model ids
    edge_params_ = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );
    // std::cerr << "create_type_id_2_syn_spec_...\n";
    create_type_id_2_syn_spec_( edge_params_ );

    try
    {

      // Open specified HDF5 edge file with read only access
      H5::H5File edge_h5_file( edge_filename, H5F_ACC_RDONLY );

      // Open top-level group (always one group named 'edges')
      H5::Group edges_group( edge_h5_file.openGroup( "edges" ) );

      // Get names of population groups
      std::vector< std::string > population_group_names;
      // https://support.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-Iterate
      H5Literate(
        edges_group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_group_names_callback, &population_group_names );


      // Iterate the population groups
      for ( const auto& population_group_name : population_group_names )
      {
        const H5::Group population_group( edges_group.openGroup( population_group_name ) );

        // Find the number of edge id groups, i.e. ones with label "0", "1", ..., by finding
        // the names of the population's datasets and subgroups
        // Note we assume edge ids are contiguous starting from zero, which is the
        // SONATA default. Edge id keys can also be custom (not handled here)
        std::vector< std::string > population_group_dset_and_subgroup_names;
        H5Literate( population_group.getId(),
          H5_INDEX_NAME,
          H5_ITER_INC,
          NULL,
          get_group_names_callback,
          &population_group_dset_and_subgroup_names );

        size_t num_edge_id_groups { 0 };
        bool is_edge_id_name;
        std::vector< std::string > edge_id_names;

        for ( const auto& name : population_group_dset_and_subgroup_names )
        {
          is_edge_id_name = ( name.find_first_not_of( "0123456789" ) == std::string::npos );

          if ( is_edge_id_name == 1 )
          {
            edge_id_names.push_back( name );
          }

          num_edge_id_groups += is_edge_id_name;
        }

        // Currently only SONATA edge files with one edge id group is supported
        if ( num_edge_id_groups == 1 )
        {
          // const auto edge_id_group = population_group.openGroup( edge_id_names[ 0 ] );
          const H5::Group edge_id_group( population_group.openGroup( edge_id_names[ 0 ] ) );

          // Check if weight and delay are given as h5 files
          is_weight_and_delay_from_dataset_( edge_id_group );

          // Open datasets
          src_node_id_dset_ = population_group.openDataSet( "source_node_id" );
          tgt_node_id_dset_ = population_group.openDataSet( "target_node_id" );
          edge_type_id_dset_ = population_group.openDataSet( "edge_type_id" );

          if ( weight_dataset_exist_ )
          {
            syn_weight_dset_ = edge_id_group.openDataSet( "syn_weight" );
          }

          if ( delay_dataset_exist_ )
          {
            delay_dset_ = edge_id_group.openDataSet( "delay" );
          }

          // Verify that source and target are equal in size
          const auto src_array_size = get_num_elements_( src_node_id_dset_ );
          const auto tgt_array_size = get_num_elements_( tgt_node_id_dset_ );
          assert( src_array_size == tgt_array_size );

          hsize_t chunk_size = CHUNK_SIZE;

          // adjust if chunk_size is too large
          if ( src_array_size < chunk_size )
          {
            chunk_size = src_array_size;
          }

          // Divide into chunks + remainder
          auto dv = std::div( static_cast< int >( src_array_size ), static_cast< int >( chunk_size ) );

          // std::cerr << "get_attributes_ source node_population...\n";
          // Retrieve source and target attributes to find which node population to map to
          get_attributes_( source_attribute_value_, src_node_id_dset_, "node_population" );
          get_attributes_( target_attribute_value_, tgt_node_id_dset_, "node_population" );


          // Iterate chunks
          hsize_t offset { 0 }; // start coordinates of data selection

          for ( size_t i { 0 }; i < dv.quot; i++ )
          {
            // create connections
            create_connections_( chunk_size, offset );

            // increment offset
            offset += chunk_size;
          }

          // Handle remainder
          if ( dv.rem > 0 )
          {
            create_connections_( dv.rem, offset );
          }

          // Close datasets
          src_node_id_dset_.close();
          src_node_id_dset_.close();
          edge_type_id_dset_.close();

          if ( weight_dataset_exist_ )
          {
            syn_weight_dset_.close();
          }

          if ( delay_dataset_exist_ )
          {
            delay_dset_.close();
          }

          // Reset all parameters
          reset_params();
        }
        else
        {
          throw NotImplemented(
            "Connecting with Sonata files with more than one edgegroup is currently not implemented" );
        }
      } // end iteration over population groups

      // Close H5 objects in scope
      edges_group.close();
      edge_h5_file.close();

    } // end try block

    // Might need more catches
    /*
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Could not open HDF5 file " + edge_filename );
    }
    */

    /*
     catch ( std::exception const& e )
     {
       std::cerr << __FILE__ << ":" << __LINE__ << " : "
                 << "Exception caught " << e.what() << "\n";
     }
     */

    catch ( const H5::Exception& e )
    {
      throw KernelException( "H5 exception caught: " + e.getDetailMsg() );
    }

    /*
    catch ( const H5::FileIException& e )
    {
      // Other:
      // H5::DataSetIException - caused by the DataSet operations
      // H5::DataSpaceIException - caused by the DataSpace operations
      // H5::DataTypeIException - caused by DataSpace operations
      std::cerr << __FILE__ << ":" << __LINE__ << " : "
                << "H5 FileIException caught:\n"
                << e.what() << "\n";
      std::cerr << __FILE__ << ":" << __LINE__ << " H5 FileIException caught: " << e.getDetailMsg() << "\n";
      e.printErrorStack();
      throw KernelException( "Failure caused by H5File operations with file " + edge_filename );
    }
    */

    catch ( ... )
    {
      std::cerr << __FILE__ << ":" << __LINE__ << " : "
                << "Unknown exception caught\n";
    }

  } // end iteration over edge files
} // end of SonataConnector::connect

hsize_t
SonataConnector::get_num_elements_( const H5::DataSet& dataset )
{
  // std::cerr << "get_num_elements_...\n";
  H5::DataSpace dataspace = dataset.getSpace();
  hsize_t dims_out[ 1 ];
  dataspace.getSimpleExtentDims( dims_out, NULL );
  // std::cerr << "dims_out: " << *dims_out << "\n";
  dataspace.close();
  return *dims_out;
}


void
SonataConnector::create_connections_( const hsize_t chunk_size, const hsize_t offset )
{


  // Read subsets
  std::vector< int > src_node_id_data_subset( chunk_size );
  std::vector< int > tgt_node_id_data_subset( chunk_size );
  std::vector< int > edge_type_id_data_subset( chunk_size );
  std::vector< double > syn_weight_data_subset( chunk_size );
  std::vector< double > delay_data_subset( chunk_size );

  read_subset_int_( src_node_id_dset_, src_node_id_data_subset, chunk_size, offset );
  read_subset_int_( tgt_node_id_dset_, tgt_node_id_data_subset, chunk_size, offset );
  read_subset_int_( edge_type_id_dset_, edge_type_id_data_subset, chunk_size, offset );

  if ( weight_dataset_exist_ )
  {
    read_subset_double_( syn_weight_dset_, syn_weight_data_subset, chunk_size, offset );
  }
  if ( delay_dataset_exist_ )
  {
    read_subset_double_( delay_dset_, syn_weight_data_subset, chunk_size, offset );
  }

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_( kernel().vp_manager.get_num_threads() );

  // std::cerr << "Enter parallel region...\n";
#pragma omp parallel
  {

    const index static_syn_model_id = kernel().model_manager.get_synapse_model_id( "static_synapse" );
    DictionaryDatum dd_empty = DictionaryDatum( new Dictionary );
    ( *dd_empty )[ "receptor_type" ] = 1;

    const auto tid = kernel().vp_manager.get_thread_id();

    // try
    //{
    //  Retrieve the correct NodeCollections
    const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
    const auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
    const auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );

    auto snode_it = current_source_nc->begin();
    auto tnode_it = current_target_nc->begin();


    // Iterate the datasets and create the connections
    for ( hsize_t i = 0; i < chunk_size; ++i )
    {

      const auto sonata_source_id = src_node_id_data_subset[ i ];
      const index snode_id = ( *( snode_it + sonata_source_id ) ).node_id;

      const auto sonata_target_id = tgt_node_id_data_subset[ i ];
      const index target_id = ( *( tnode_it + sonata_target_id ) ).node_id;
      Node* target = kernel().node_manager.get_node_or_proxy( target_id );

      /*
        if ( not target ) // TODO: remove
        {
#pragma omp critical
          {
            std::cerr << kernel().vp_manager.get_thread_id() << ": " << target_id << " node_or_proxy is NULL!\n";
          }
        }
        */

      const thread target_thread = target->get_thread();

      // Skip if target is not on this thread, or not on this MPI process.
      if ( target->is_proxy() or tid != target_thread )
      {
        continue;
      }

      /*
        const auto edge_type_id = edge_type_id_data_subset[ i ];
        const auto syn_spec = getValue< DictionaryDatum >( edge_params_->lookup( std::to_string( edge_type_id ) ) );

        auto get_syn_property = [ &syn_spec, &i ]( const bool dataset, std::vector< double >& data, const Name& name )
        {
          if ( dataset ) // Syn_property is set from dataset if the dataset is defined
          {
            if ( not data[ i ] ) // TODO: remove
            {
#pragma omp critical
              {
                // //std::cerr << kernel().vp_manager.get_thread_id() << ": " << name << " " << i
                //          << " data index is NULL!\n";
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

        const double weight = get_syn_property( weight_dataset_exist_, syn_weight_data_subset, names::weight );
        const double delay = get_syn_property( delay_dataset_exist_, delay_data_subset, names::delay );

        RngPtr rng = get_vp_specific_rng( target_thread );
        get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

        kernel().connection_manager.connect( snode_id,
        target,
        target_thread,
        type_id_2_syn_model_.at( edge_type_id ),             // static synapse
        type_id_2_param_dicts_.at( edge_type_id ).at( tid ), // send empty param dict
        delay,                                               // fixed as 1
        weight );
        */


      const double delay_fixed = 1.0;
      const double weight_fixed = 1.0;


      kernel().connection_manager.connect(
        snode_id, target, target_thread, static_syn_model_id, dd_empty, delay_fixed, weight_fixed );
    }
    //}
    /*
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
    */

  } // omp parallel

  // check if any exceptions have been raised
  for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
  {
    if ( exceptions_raised_.at( thr ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
    }
  }
}


void
SonataConnector::read_subset_int_( const H5::DataSet& dataset,
  std::vector< int >& data_buf,
  hsize_t chunk_size,
  hsize_t offset )
{
  // Define Memory Dataspace. Get file dataspace and select
  // hyperslab from the file dataspace. In call for selecting
  // hyperslab 'stride' and 'block' are implicitly given as NULL.
  // H5S_SELECT_SET replaces any existing selection with the parameters
  // from this call
  const int array_dim = 1;
  H5::DataSpace memspace( array_dim, &chunk_size, NULL );
  H5::DataSpace dataspace = dataset.getSpace();
  dataspace.selectHyperslab( H5S_SELECT_SET, &chunk_size, &offset );

  // Read subset
  dataset.read( data_buf.data(), H5::PredType::NATIVE_INT, memspace, dataspace );

  // Close dataspaces
  dataspace.close();
  memspace.close();
}

void
SonataConnector::read_subset_double_( const H5::DataSet& dataset,
  std::vector< double >& data_buf,
  hsize_t chunk_size,
  hsize_t offset )
{
  // Define Memory Dataspace. Get file dataspace and select
  // hyperslab from the file dataspace. In call for selecting
  // hyperslab 'stride' and 'block' are implicitly given as NULL.
  // H5S_SELECT_SET replaces any existing selection with the parameters
  // from this call
  const int array_dim = 1;
  H5::DataSpace memspace( array_dim, &chunk_size, NULL );
  H5::DataSpace dataspace = dataset.getSpace();
  dataspace.selectHyperslab( H5S_SELECT_SET, &chunk_size, &offset );

  // Read subset
  dataset.read( data_buf.data(), H5::PredType::NATIVE_DOUBLE, memspace, dataspace );

  // Close dataspaces
  dataspace.close();
  memspace.close();
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
SonataConnector::is_weight_and_delay_from_dataset_( const H5::Group& group )
{
  weight_dataset_exist_ = H5Lexists( group.getId(), "syn_weight", H5P_DEFAULT ) > 0;
  delay_dataset_exist_ = H5Lexists( group.getId(), "delay", H5P_DEFAULT ) > 0;
}

void
SonataConnector::create_type_id_2_syn_spec_( DictionaryDatum edge_params )
{
  for ( auto it = edge_params->begin(); it != edge_params->end(); ++it )
  {
    const auto type_id = std::stoi( it->first.toString() );
    auto d = getValue< DictionaryDatum >( it->second );
    const auto syn_name = getValue< std::string >( ( *d )[ "synapse_model" ] );
    // std::cerr << "type_id=" << type_id << " syn_name=" << syn_name << "\n";

    // The following call will throw "UnknownSynapseType" if syn_name is not naming a known model
    const index synapse_model_id = kernel().model_manager.get_synapse_model_id( syn_name );
    // std::cerr << "synapse_model_id=" << synapse_model_id << "\n";

    set_synapse_params( d, synapse_model_id, type_id );
    // std::cerr << "synapse_model_id=" << synapse_model_id << "\n";
    type_id_2_syn_model_[ type_id ] = synapse_model_id;
  }
}

void
SonataConnector::set_synapse_params( DictionaryDatum syn_dict, index synapse_model_id, int type_id )
{
  // std::cerr << "set_synapse_params...\n";
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
    // std::cerr << "type_id_2_syn_spec_.at...\n";
    type_id_2_syn_spec_[ type_id ].push_back( synapse_params ); // DO WE NEED TO DEFINE THIS PER THREAD???
    // std::cerr << "type_id_2_param_dicts_.at...\n";
    type_id_2_param_dicts_[ type_id ].push_back( new Dictionary );

    for ( auto param : synapse_params )
    {
      if ( param.second->provides_long() )
      {
        // std::cerr << "int type_id_2_param_dicts_.at...\n";
        ( *type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new IntegerDatum( 0 ) );
      }
      else
      {
        // std::cerr << "double type_id_2_param_dicts_.at...\n";
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
get_group_names_callback( hid_t loc_id, const char* name, const H5L_info_t*, void* opdata )
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
