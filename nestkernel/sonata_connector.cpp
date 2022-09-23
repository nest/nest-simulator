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
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

#include <chrono>   // for debugging
#include <fstream>  // for debugging
#include <iostream> // for debugging

#include "H5Cpp.h" // HDF5 C++ API header file

extern "C" herr_t get_group_names_callback( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

// constexpr hsize_t CHUNK_SIZE = 10000;      // 1e4
// constexpr hsize_t CHUNK_SIZE = 100000;     // 1e5
// constexpr hsize_t CHUNK_SIZE = 1000000;    // 1e6
// constexpr hsize_t CHUNK_SIZE = 10000000;   // 1e7
// constexpr hsize_t CHUNK_SIZE = 100000000;  // 1e8
constexpr hsize_t CHUNK_SIZE = 1000000000; // 1e9

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


  const auto rank = kernel().mpi_manager.get_rank();
  const std::string perf_filename = "perf_" + std::to_string( rank ) + ".txt";
  const std::string dbg_filename = "dbg_" + std::to_string( rank ) + ".txt";
  std::ofstream perf_file( perf_filename );
  std::ofstream dbg_file( dbg_filename );

  dbg_file << "\nREAD AND CONNECT EDGES\n\n";

  /*
    // timer start
    const auto X_start_t = std::chrono::high_resolution_clock::now();

    // timer end
      const auto X_end_t = std::chrono::high_resolution_clock::now();
      const std::chrono::duration< double, std::milli > X_tot_t = X_end_t - X_start_t;
      perf_file << "X: " << x_tot_t.count() << "ms\n";
    */


  // timer start
  const auto connect_start_t = std::chrono::high_resolution_clock::now();

  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );

  for ( auto edge_dictionary_datum : edges )
  {

    const auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    const auto edge_filename = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    perf_file << "\nCurrent file: " << edge_filename << "\n";
    dbg_file << "Current file: " << edge_filename << "\n";

    // Create map of edge type ids to NEST synapse_model ids
    edge_params_ = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );
    // std::cerr << "create_type_id_2_syn_spec_...\n";
    create_type_id_2_syn_spec_( edge_params_ );


    try
    {
      // timer start
      const auto load_file_start_t = std::chrono::high_resolution_clock::now();

      // Open specified HDF5 edge file with read only access
      H5::H5File edge_h5_file( edge_filename, H5F_ACC_RDONLY );

      // timer end
      const auto load_file_end_t = std::chrono::high_resolution_clock::now();
      const std::chrono::duration< double, std::milli > load_file_tot_t = load_file_end_t - load_file_start_t;
      perf_file << "Open file: " << load_file_tot_t.count() << "ms\n";


      // timer start
      const auto load_edges_group_start_t = std::chrono::high_resolution_clock::now();

      // Open top-level group (always one group named 'edges')
      H5::Group edges_group( edge_h5_file.openGroup( "edges" ) );

      // timer end
      const auto load_edges_group_end_t = std::chrono::high_resolution_clock::now();
      const std::chrono::duration< double, std::milli > load_edges_group_tot_t =
        load_edges_group_end_t - load_edges_group_start_t;
      perf_file << "Open 'edges' group: " << load_edges_group_tot_t.count() << "ms\n";

      // timer start
      const auto get_population_names_start_t = std::chrono::high_resolution_clock::now();

      // Get names of population groups
      std::vector< std::string > population_group_names;
      // https://support.hdfgroup.org/HDF5/doc/RM/RM_H5L.html#Link-Iterate
      H5Literate(
        edges_group.getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_group_names_callback, &population_group_names );

      // timer end
      const auto get_population_names_end_t = std::chrono::high_resolution_clock::now();
      const std::chrono::duration< double, std::milli > get_population_names_tot_t =
        get_population_names_end_t - get_population_names_start_t;
      perf_file << "Get population names: " << get_population_names_tot_t.count() << "ms\n";


      // Iterate the population groups
      for ( const auto& population_group_name : population_group_names )
      {

        // timer start
        const auto load_population_group_start_t = std::chrono::high_resolution_clock::now();

        const H5::Group population_group( edges_group.openGroup( population_group_name ) );

        // timer end
        const auto load_population_group_end_t = std::chrono::high_resolution_clock::now();
        const std::chrono::duration< double, std::milli > load_population_group_tot_t =
          load_population_group_end_t - load_population_group_start_t;
        perf_file << "Open population group: " << load_population_group_tot_t.count() << "ms\n";

        // timer start
        const auto find_num_edge_id_groups_start_t = std::chrono::high_resolution_clock::now();

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

        // timer end
        const auto find_num_edge_id_groups_end_t = std::chrono::high_resolution_clock::now();
        const std::chrono::duration< double, std::milli > find_num_edge_id_groups_tot_t =
          find_num_edge_id_groups_end_t - find_num_edge_id_groups_start_t;
        perf_file << "Find num_edge_id_groups: " << find_num_edge_id_groups_tot_t.count() << "ms\n";

        // Currently only SONATA edge files with one edge id group is supported
        if ( num_edge_id_groups == 1 )
        {

          // timer start
          const auto load_edge_id_group_start_t = std::chrono::high_resolution_clock::now();

          // const auto edge_id_group = population_group.openGroup( edge_id_names[ 0 ] );
          const H5::Group edge_id_group( population_group.openGroup( edge_id_names[ 0 ] ) );

          // timer end
          const auto load_edge_id_group_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > load_edge_id_group_tot_t =
            load_edge_id_group_end_t - load_edge_id_group_start_t;
          perf_file << "Open edge id group: " << load_edge_id_group_tot_t.count() << "ms\n";


          // timer start
          const auto is_weight_and_delay_start_t = std::chrono::high_resolution_clock::now();

          // Check if weight and delay are given as h5 files
          is_weight_and_delay_from_dataset_( edge_id_group );

          // timer end
          const auto is_weight_and_delay_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > is_weight_and_delay_tot_t =
            is_weight_and_delay_end_t - is_weight_and_delay_start_t;
          perf_file << "Check if weight and delay dsets exist: " << is_weight_and_delay_tot_t.count() << "ms\n";

          // timer start
          const auto load_dsets_start_t = std::chrono::high_resolution_clock::now();

          // Open datasets
          src_node_id_dset_ = population_group.openDataSet( "source_node_id" );
          tgt_node_id_dset_ = population_group.openDataSet( "target_node_id" );
          edge_type_id_dset_ = population_group.openDataSet( "edge_type_id" );

          // timer end
          const auto load_dsets_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > load_dsets_tot_t = load_dsets_end_t - load_dsets_start_t;
          perf_file << "Open src, tgt and edge type id dsets: " << load_dsets_tot_t.count() << "ms\n";

          // timer start
          const auto load_w_and_d_dsets_start_t = std::chrono::high_resolution_clock::now();

          if ( weight_dataset_exist_ )
          {
            syn_weight_dset_ = edge_id_group.openDataSet( "syn_weight" );
          }

          if ( delay_dataset_exist_ )
          {
            delay_dset_ = edge_id_group.openDataSet( "delay" );
          }

          // timer end
          const auto load_w_and_d_dsets_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > load_w_and_d_dsets_tot_t =
            load_w_and_d_dsets_end_t - load_w_and_d_dsets_start_t;
          perf_file << "Open weight and/or delay dsets if present: " << load_w_and_d_dsets_tot_t.count() << "ms\n";

          // timer start
          const auto set_chunk_size_start_t = std::chrono::high_resolution_clock::now();

          // make sure that target and source population have the same size
          const auto src_array_size = get_num_elements_( src_node_id_dset_ );
          const auto tgt_array_size = get_num_elements_( tgt_node_id_dset_ );

          // assert( src_array_size == tgt_array_size );
          if ( src_array_size != tgt_array_size )
          {
            throw DimensionMismatch( "Source and Target population must be of the same size." );
          }

          hsize_t chunk_size = CHUNK_SIZE;

          // adjust if chunk_size is too large
          if ( src_array_size < chunk_size )
          {
            chunk_size = src_array_size;
          }

          dbg_file << "d_set size: " << src_array_size << "\n";
          dbg_file << "Chunk size: " << chunk_size << "\n";

          // Divide into chunks + remainder
          // cast to unsigned long long?
          // https://learn.microsoft.com/en-us/cpp/c-language/cpp-integer-limits?view=msvc-170
          auto dv = std::div( static_cast< long >( src_array_size ), static_cast< long >( chunk_size ) );

          dbg_file << "Split into chunks + remainder\n";
          dbg_file << "  -> n_chunks: " << dv.quot << "\n";
          dbg_file << "  -> remainder: " << dv.rem << "\n";

          // timer end
          const auto set_chunk_size_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > set_chunk_size_tot_t =
            set_chunk_size_end_t - set_chunk_size_start_t;
          perf_file << "Set chunk size: " << set_chunk_size_tot_t.count() << "ms\n";

          // timer start
          const auto get_attributes_start_t = std::chrono::high_resolution_clock::now();

          // std::cerr << "get_attributes_ source node_population...\n";
          // Retrieve source and target attributes to find which node population to map to
          get_attributes_( source_attribute_value_, src_node_id_dset_, "node_population" );
          get_attributes_( target_attribute_value_, tgt_node_id_dset_, "node_population" );

          // timer end
          const auto get_attributes_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > get_attributes_tot_t =
            get_attributes_end_t - get_attributes_start_t;
          perf_file << "Get src and tgt attributes: " << get_attributes_tot_t.count() << "ms\n";


          // timer start
          const auto iter_chunks_start_t = std::chrono::high_resolution_clock::now();

          // Iterate chunks
          hsize_t offset { 0 }; // start coordinates of data selection


          // Define non-parallel stopwatches
          Stopwatch create_arrays_stopwatch;
          Stopwatch read_subsets_stopwatch;

          // Define vectors for storing parallel stopwatch times
          const auto n_threads = kernel().vp_manager.get_num_threads();
          auto connect_times = std::vector< double >( n_threads, 0 );
          auto thread_organ_times = std::vector< double >( n_threads, 0 );
          auto get_node_ids_times = std::vector< double >( n_threads, 0 );
          auto thread_check_times = std::vector< double >( n_threads, 0 );
          auto synapse_prop_times = std::vector< double >( n_threads, 0 );

          perf_file << "= Iterating chunks =\n";

          for ( size_t i = 0; i < dv.quot; i++ )
          {
            // create connections
            // create_connections_( chunk_size, offset );
            dbg_file << "Chunk iteration " << i << "\n";


            //-------------------------------------------------------------------------------------------------------------
            // BEGIN
            //-------------------------------------------------------------------------------------------------------------

            dbg_file << "Create arrays\n";
            // Start stopwatch
            create_arrays_stopwatch.start();

            // Create arrays
            std::vector< int > src_node_id_data_subset( chunk_size );
            std::vector< int > tgt_node_id_data_subset( chunk_size );
            std::vector< int > edge_type_id_data_subset( chunk_size );
            std::vector< double > syn_weight_data_subset( chunk_size );
            std::vector< double > delay_data_subset( chunk_size );

            // Stop stopwatch
            create_arrays_stopwatch.stop();

            // Start stopwatch
            read_subsets_stopwatch.start();

            dbg_file << "Read subsets\n";
            // Read subsets
            read_subset_int_( src_node_id_dset_, src_node_id_data_subset, chunk_size, offset );
            dbg_file << "  -> Read src id dset success\n";
            read_subset_int_( tgt_node_id_dset_, tgt_node_id_data_subset, chunk_size, offset );
            dbg_file << "  -> Read tgt id dset success\n";
            read_subset_int_( edge_type_id_dset_, edge_type_id_data_subset, chunk_size, offset );
            dbg_file << "  -> Read edge type id dset success\n";

            if ( weight_dataset_exist_ )
            {
              dbg_file << "  -> Weight dset exists\n";
              read_subset_double_( syn_weight_dset_, syn_weight_data_subset, chunk_size, offset );
              dbg_file << "  -> Read weight dset success\n";
            }
            if ( delay_dataset_exist_ )
            {
              dbg_file << "  -> Delay dset exists\n";
              read_subset_double_( delay_dset_, syn_weight_data_subset, chunk_size, offset );
              dbg_file << "  -> Read delay dset success\n";
            }

            // Stop stopwatch
            read_subsets_stopwatch.stop();

            std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_(
              kernel().vp_manager.get_num_threads() );

            dbg_file << "Enter parallel region\n";
#pragma omp parallel
            {

              Stopwatch connect_stopwatch;
              Stopwatch thread_organ_stopwatch;
              Stopwatch get_node_ids_stopwatch;
              Stopwatch thread_check_stopwatch;
              Stopwatch synapse_prop_stopwatch;

              // thread and iterator organization
              const auto tid = kernel().vp_manager.get_thread_id();
              // const auto vp = kernel().vp_manager.thread_to_vp( tid );  // håkon
              RngPtr rng = get_vp_specific_rng( tid ); // moved outside of connect loop

              // start stopwatch
              thread_organ_stopwatch.start();
              // does this have to be inside the parallel region?
              const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
              const auto current_source_nc =
                getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
              const auto current_target_nc =
                getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );

              auto snode_it = current_source_nc->begin();
              auto tnode_it = current_target_nc->begin();

              // stop stopwatch
              thread_organ_stopwatch.stop();

              // Iterate the datasets and create the connections
              for ( size_t i = 0; i < chunk_size; ++i )
              {

                // Start stopwatch
                get_node_ids_stopwatch.start();

                const auto sonata_source_id = src_node_id_data_subset[ i ];
                const auto sonata_target_id = tgt_node_id_data_subset[ i ];
                const index snode_id = ( *( snode_it + sonata_source_id ) ).node_id;
                // const index tnode_id = ( *( tnode_it + sonata_target_id ) ).node_id;
                //  håkon
                const auto target_triple = *( tnode_it + sonata_target_id );
                const index tnode_id = target_triple.node_id;

                // Stop stopwatch
                // get_node_ids_watches[ tid ].stop();
                get_node_ids_stopwatch.stop();

                // thread check timer
                // Start stopwatch
                thread_check_stopwatch.start();

                // newest version
                // Check whether the target is on this mpi machine
                if ( not kernel().node_manager.is_local_node_id( tnode_id ) )
                {
                  continue;
                }


                Node* target = kernel().node_manager.get_node_or_proxy( tnode_id );
                const thread target_thread = target->get_thread();

                // check whether target is on our thread
                if ( tid != target_thread )
                {
                  continue;
                }


                // Stop stopwatch
                thread_check_stopwatch.stop();

                // synapse timer
                // Start stopwatch
                synapse_prop_stopwatch.start();

                const auto edge_type_id = edge_type_id_data_subset[ i ];
                const auto syn_spec =
                  getValue< DictionaryDatum >( edge_params_->lookup( std::to_string( edge_type_id ) ) );

                auto get_syn_property = [ &syn_spec, &i ](
                                          const bool dataset, std::vector< double >& data, const Name& name )
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

                // RngPtr rng = get_vp_specific_rng( target_thread );  // moved outside of loop since we verify that tid
                // == target_thread
                get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

                // Stop stopwatch
                synapse_prop_stopwatch.stop();

                // Start stopwatch
                connect_stopwatch.start();

                kernel().connection_manager.connect( snode_id,
                  target,
                  target_thread,
                  type_id_2_syn_model_.at( edge_type_id ),
                  type_id_2_param_dicts_.at( edge_type_id ).at( tid ),
                  delay,
                  weight );

                // Stop stopwatch
                connect_stopwatch.stop();

              } // end for

              // stopwatch fixpoint
              thread_organ_times[ tid ] += thread_organ_stopwatch.elapsed( Stopwatch::MILLISEC );
              get_node_ids_times[ tid ] += get_node_ids_stopwatch.elapsed( Stopwatch::MILLISEC );
              thread_check_times[ tid ] += thread_check_stopwatch.elapsed( Stopwatch::MILLISEC );
              synapse_prop_times[ tid ] += synapse_prop_stopwatch.elapsed( Stopwatch::MILLISEC );
              connect_times[ tid ] += connect_stopwatch.elapsed( Stopwatch::MILLISEC );


            } // omp parallel
            dbg_file << "Exit parallel region\n\n";

            // check if any exceptions have been raised
            for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
            {
              if ( exceptions_raised_.at( thr ).get() )
              {
                throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
              }
            }

            //--------------------------------------------------------------------------------------------------------------
            // END
            //--------------------------------------------------------------------------------------------------------------


            // increment offset
            offset += chunk_size;
          } // end chunk loop


          // Fix stopwatches here
          auto write_min_max_avg = [ &perf_file ]( std::vector< double > times, std::string name )
          {
            const auto min_element = std::min_element( times.begin(), times.end() );
            const auto max_element = std::max_element( times.begin(), times.end() );
            const auto avg = std::accumulate( times.begin(), times.end(), 0.0 ) / times.size();
            perf_file << name << " min, max, avg:\t " << *min_element << "ms, " << *max_element << "ms, " << avg
                      << "ms\n";
          };

          perf_file << "-> Create arrays: " << create_arrays_stopwatch.elapsed( Stopwatch::MILLISEC ) << "ms\n";
          perf_file << "-> Read subsets: " << read_subsets_stopwatch.elapsed( Stopwatch::MILLISEC ) << "ms\n";

          // Handle parallel stopwatches
          perf_file << "   = Enter parallel region =\n";

          // Write parallel times to dbg file
          write_min_max_avg( thread_organ_times, "-> thread and iterator organization " );
          perf_file << "   = Iterate dataset and connect =\n";
          write_min_max_avg( get_node_ids_times, "-> get node ids " );
          write_min_max_avg( thread_check_times, "-> is target vp this vp " );
          write_min_max_avg( synapse_prop_times, "-> synapse properties " );
          write_min_max_avg( connect_times, "-> connect edges " );


          // timer end
          const auto iter_chunks_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > iter_chunks_tot_t = iter_chunks_end_t - iter_chunks_start_t;
          perf_file << "Iterate chunks tot time: " << iter_chunks_tot_t.count() << "ms\n";


          // timer start
          const auto iter_remainder_start_t = std::chrono::high_resolution_clock::now();

          dbg_file << "Check if remainder\n";
          // Handle remainder
          if ( dv.rem > 0 )
          {
            dbg_file << "Connect remainder\n";
            create_connections_( dv.rem, offset );
            dbg_file << "Done with remainder\n";
          }

          // timer end
          const auto iter_remainder_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > iter_remainder_tot_t =
            iter_remainder_end_t - iter_remainder_start_t;
          perf_file << "Iterate possible remainder: " << iter_remainder_tot_t.count() << "ms\n";


          // timer start
          const auto close_reset_start_t = std::chrono::high_resolution_clock::now();

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


          // timer end
          const auto close_reset_end_t = std::chrono::high_resolution_clock::now();
          const std::chrono::duration< double, std::milli > close_reset_tot_t = close_reset_end_t - close_reset_start_t;
          perf_file << "Close dsets and reset: " << close_reset_tot_t.count() << "ms\n";
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

  // timer end
  const auto connect_end_t = std::chrono::high_resolution_clock::now();
  const std::chrono::duration< double, std::milli > connect_tot_t = connect_end_t - connect_start_t;
  perf_file << "\nConnect(): " << connect_tot_t.count() << "ms\n";
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

#pragma omp parallel
  {
    const auto tid = kernel().vp_manager.get_thread_id();
    RngPtr rng = get_vp_specific_rng( tid );

    try
    {
      // Retrieve the correct NodeCollections
      // Does this have to be inside the parallel region?
      const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
      const auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
      const auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );

      auto snode_it = current_source_nc->begin();
      auto tnode_it = current_target_nc->begin();

      // Iterate the datasets and create the connections
      for ( hsize_t i = 0; i < chunk_size; ++i )
      {
        const auto sonata_source_id = src_node_id_data_subset[ i ];
        const auto sonata_target_id = tgt_node_id_data_subset[ i ];
        const index snode_id = ( *( snode_it + sonata_source_id ) ).node_id;
        const index tnode_id = ( *( tnode_it + sonata_target_id ) ).node_id;

        // Check whether the target is on this mpi machine
        if ( not kernel().node_manager.is_local_node_id( tnode_id ) )
        {
          continue;
        }

        Node* target = kernel().node_manager.get_node_or_proxy( tnode_id );
        const thread target_thread = target->get_thread();

        // check whether target is on our thread
        if ( tid != target_thread )
        {
          continue;
        }

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

        // RngPtr rng = get_vp_specific_rng( target_thread );
        get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

        kernel().connection_manager.connect( snode_id,
          target,
          target_thread,
          type_id_2_syn_model_.at( edge_type_id ),             // static synapse
          type_id_2_param_dicts_.at( edge_type_id ).at( tid ), // send empty param dict
          delay,                                               // fixed as 1
          weight );

      } // end for
    }   // end try

    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }

  } // end parallel region

  // check if any exceptions have been raised
  for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
  {
    if ( exceptions_raised_.at( thr ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
    }
  }
} // end create_connections_()


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
  H5::DataSpace dataspace = dataset.getSpace();
  // Get the number of dimensions in the dataspace.
  int array_dim = dataspace.getSimpleExtentNdims(); // should maybe assert if array_dim == 1
  dataspace.selectHyperslab( H5S_SELECT_SET, &chunk_size, &offset );
  H5::DataSpace memspace( array_dim, &chunk_size, NULL );

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
  H5::DataSpace dataspace = dataset.getSpace();
  // Get the number of dimensions in the dataspace.
  int array_dim = dataspace.getSimpleExtentNdims(); // should maybe assert if array_dim == 1
  dataspace.selectHyperslab( H5S_SELECT_SET, &chunk_size, &offset );
  H5::DataSpace memspace( array_dim, &chunk_size, NULL );

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
