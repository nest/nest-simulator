/*
 *  connection_creator.cpp
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

#include "connection_creator.h"

#include "nest.h"
#include "spatial.h"

namespace nest
{

ConnectionCreator::ConnectionCreator( const Dictionary& dict )
  : allow_autapses_( true )
  , allow_multapses_( true )
  , allow_oversized_( false )
  , number_of_connections_()
  , mask_()
  , kernel_()
  , synapse_model_()
  , weight_()
  , delay_()
{
  std::string connection_type;

  dict.update_value( names::connection_type, connection_type );
  dict.update_value( names::allow_autapses, allow_autapses_ );
  dict.update_value( names::allow_multapses, allow_multapses_ );
  dict.update_value( names::allow_oversized_mask, allow_oversized_ );

  // Need to store number of connections in a temporary variable to be able to detect negative values.
  if ( dict.known( names::number_of_connections ) )
  {
    if ( is_type< ParameterPTR >( dict.at( names::number_of_connections ) ) )
    {
      dict.update_value< ParameterPTR >( names::number_of_connections, number_of_connections_ );
    }
    else
    {
      // Assume indegree is a scalar.
      const long value = dict.get< long >( names::number_of_connections );
      if ( value < 0 )
      {
        throw BadProperty( "Number of connections cannot be less than zero." );
      }
      number_of_connections_ = ParameterPTR( new ConstantParameter( value ) );
    }
  }
  if ( dict.known( names::mask ) )
  {
    mask_ = create_mask( dict.get< Dictionary >( names::mask ) );
  }
  if ( dict.known( names::kernel ) )
  {
    kernel_ = create_parameter( dict.at( names::kernel ) );
  }

  if ( dict.known( names::synapse_parameters ) )
  {
    // If synapse_parameters exists, we have collocated synapses.
    std::vector< Dictionary > syn_params_dvd;

    try
    {
      dict.update_value( names::synapse_parameters, syn_params_dvd );
    }
    catch ( const nest::TypeMismatch& )
    {
      // Give a more helpful message if the provided type is wrong.
      throw BadProperty( "synapse_parameters must be list of dictionaries" );
    }

    param_dicts_.resize( syn_params_dvd.size() );
    auto param_dict = param_dicts_.begin();
    for ( auto syn_param_it = syn_params_dvd.begin(); syn_param_it < syn_params_dvd.end();
          ++syn_param_it, ++param_dict )
    {
      extract_params_( *syn_param_it, *param_dict );
    }
  }
  else
  {
    // If not, we have single synapses.
    param_dicts_.resize( 1 );
    param_dicts_[ 0 ].resize( kernel().vp_manager.get_num_threads() );
    extract_params_( dict, param_dicts_[ 0 ] );
  }

  // We check if dict has been read out completely to stop the process early in case of errors
  dict.all_entries_accessed( "synapse_specs", "ConnectionCreator" );

  // Set default synapse_model, weight and delay if not given explicitly
  if ( synapse_model_.empty() )
  {
    synapse_model_ = { kernel().model_manager.get_synapse_model_id( "static_synapse" ) };
  }
  Dictionary syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_[ 0 ] );
  if ( weight_.empty() )
  {
    weight_ = { create_parameter( syn_defaults[ names::weight ] ) };
  }
  if ( delay_.empty() )
  {
    if ( not syn_defaults.get< bool >( names::has_delay ) )
    {
      delay_ = { create_parameter( numerics::nan ) };
    }
    else
    {
      delay_ = { create_parameter( syn_defaults[ names::delay ] ) };
    }
  }

  if ( connection_type == names::pairwise_bernoulli_on_source )
  {
    if ( dict.known( names::number_of_connections ) )
    {
      type_ = Fixed_indegree;
    }
    else
    {
      type_ = Pairwise_bernoulli_on_source;
    }
  }
  else if ( connection_type == names::pairwise_poisson )
  {
    type_ = Pairwise_poisson;
  }
  else if ( connection_type == names::pairwise_bernoulli_on_target )
  {
    if ( dict.known( names::number_of_connections ) )
    {
      type_ = Fixed_outdegree;
    }
    else
    {
      type_ = Pairwise_bernoulli_on_target;
    }
  }
  else
  {
    throw BadProperty( "Unknown connection type." );
  }
}

void
ConnectionCreator::extract_params_( const Dictionary& dict, std::vector< Dictionary >& params )
{
  const std::string syn_name = dict.known( names::synapse_model ) ? dict.get< std::string >( names::synapse_model )
                                                                  : std::string( "static_synapse" );

  // The following call will throw "UnknownSynapseType" if syn_name is not naming a known model
  const size_t synapse_model_id = kernel().model_manager.get_synapse_model_id( syn_name );
  synapse_model_.push_back( synapse_model_id );

  Dictionary syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_id );
  if ( dict.known( names::weight ) )
  {
    weight_.push_back( create_parameter( dict.at( names::weight ) ) );
  }
  else
  {
    weight_.push_back( create_parameter( syn_defaults[ names::weight ] ) );
  }

  if ( dict.known( names::delay ) )
  {
    delay_.push_back( create_parameter( dict.at( names::delay ) ) );
  }
  else
  {
    if ( not syn_defaults.get< bool >( names::has_delay ) )
    {
      delay_.push_back( create_parameter( numerics::nan ) );
    }
    else
    {
      delay_.push_back( create_parameter( syn_defaults[ names::delay ] ) );
    }
  }

  Dictionary syn_dict;
  // Using a lambda function here instead of updateValue because updateValue causes
  // problems when setting a value to a dictionary-entry in syn_dict.
  auto copy_long_if_known = [ &syn_dict, &dict ]( const std::string& name ) -> void
  {
    if ( dict.known( name ) )
    {
      syn_dict[ name ] = dict.get< long >( name );
    }
  };
  copy_long_if_known( names::synapse_label );
  copy_long_if_known( names::receptor_type );

  params.resize( kernel().vp_manager.get_num_threads() );
#pragma omp parallel
  {
    params.at( kernel().vp_manager.get_thread_id() ) = syn_dict;
  }
}

} // namespace nest
