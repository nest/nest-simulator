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

namespace nest
{

ConnectionCreator::ConnectionCreator( dictionary dict )
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
  Name connection_type;
  long number_of_connections( -1 ); // overwritten by dict entry

  dict.update_value( names::connection_type, connection_type );
  dict.update_value( names::allow_autapses, allow_autapses_ );
  dict.update_value( names::allow_multapses, allow_multapses_ );
  dict.update_value( names::allow_oversized_mask, allow_oversized_ );

  // Need to store number of connections in a temporary variable to be able to detect negative values.
  if ( dict.update_value( names::number_of_connections, number_of_connections ) )
  {
    if ( number_of_connections < 0 )
    {
      throw BadProperty( "Number of connections cannot be less than zero." );
    }
    // We are sure that number of connections isn't negative, so it is safe to store it in a size_t.
    number_of_connections_ = number_of_connections;
  }
  // TODO-PYNEST-NG: implement mask with dictionary
  // if ( dict.known( names::mask ) )
  // {
  //   mask_ = NestModule::create_mask( ( *dict )[ names::mask ] );
  // }
  if ( dict.known( names::kernel ) )
  {
    kernel_ = create_parameter( dict[ names::kernel ] );
  }
  // TODO-PYNEST-NG: collocated synapses
  // if ( dict.known( names::synapse_parameters ) )
  // {
  //   // If synapse_parameters exists, we have collocated synapses.
  //   ArrayDatum* syn_params_dvd =
  //     dynamic_cast< ArrayDatum* >( ( *dict )[ names::synapse_parameters ].datum() );
  //   if ( not syn_params_dvd )
  //   {
  //     throw BadProperty( "synapse_parameters must be list of dictionaries" );
  //   }

  //   param_dicts_.resize( syn_params_dvd->size() );
  //   auto param_dict = param_dicts_.begin();
  //   for ( auto synapse_datum = syn_params_dvd->begin(); synapse_datum < syn_params_dvd->end();
  //         ++synapse_datum, ++param_dict )
  //   {
  //     auto syn_param = dynamic_cast< dictionary* >( synapse_datum->datum() );
  //     extract_params_( *syn_param, *param_dict );
  //   }
  // }
  // else
  // {
  // If not, we have single synapses.
  param_dicts_.resize( 1 );
  param_dicts_[ 0 ].resize( kernel().vp_manager.get_num_threads() );
  extract_params_( dict, param_dicts_[ 0 ] );
  // }

  // Set default synapse_model, weight and delay if not given explicitly
  if ( synapse_model_.empty() )
  {
    synapse_model_ = { kernel().model_manager.get_synapsedict().get< synindex >( "static_synapse" ) };
  }
  dictionary syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_[ 0 ] );
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

    if ( number_of_connections >= 0 )
    {
      type_ = Fixed_indegree;
    }
    else
    {
      type_ = Pairwise_bernoulli_on_source;
    }
  }
  else if ( connection_type == names::pairwise_bernoulli_on_target )
  {

    if ( number_of_connections >= 0 )
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
ConnectionCreator::extract_params_( dictionary& dict_datum, std::vector< dictionary >& params )
{
  if ( not dict_datum.known( names::synapse_model ) )
  {
    dict_datum[ names::synapse_model ] = "static_synapse";
  }
  const std::string syn_name = dict_datum.get< std::string >( names::synapse_model );

  if ( not kernel().model_manager.get_synapsedict().known( syn_name ) )
  {
    throw UnknownSynapseType( syn_name );
  }
  index synapse_model_id = kernel().model_manager.get_synapsedict().get< synindex >( syn_name );
  synapse_model_.push_back( synapse_model_id );

  dictionary syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_id );
  if ( dict_datum.known( names::weight ) )
  {
    weight_.push_back( create_parameter( dict_datum[ names::weight ] ) );
  }
  else
  {
    weight_.push_back( create_parameter( syn_defaults[ names::weight ] ) );
  }

  if ( dict_datum.known( names::delay ) )
  {
    delay_.push_back( create_parameter( dict_datum[ names::delay ] ) );
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

  dictionary syn_dict;
  // Using a lambda function here instead of updateValue because updateValue causes
  // problems when setting a value to a dictionary-entry in syn_dict.
  auto copy_long_if_known = [&syn_dict, &dict_datum]( const std::string& name ) -> void {
    if ( dict_datum.known( name ) )
    {
      syn_dict[ name ] = dict_datum.get< long >( name );
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
