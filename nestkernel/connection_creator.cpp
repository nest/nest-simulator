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

namespace nest
{

ConnectionCreator::ConnectionCreator( DictionaryDatum dict )
  : allow_autapses_( true )
  , allow_multapses_( true )
  , allow_oversized_( false )
  , number_of_connections_()
  , mask_()
  , kernel_()
  , synapse_model_()
  , weight_()
  , delay_()
  , dummy_param_dicts_()
{
  Name connection_type;
  long number_of_connections( -1 ); // overwritten by dict entry

  for ( Dictionary::iterator dit = dict->begin(); dit != dict->end(); ++dit )
  {

    if ( dit->first == names::connection_type )
    {
      connection_type = getValue< std::string >( dit->second );
    }
    else if ( dit->first == names::allow_autapses )
    {
      allow_autapses_ = getValue< bool >( dit->second );
    }
    else if ( dit->first == names::allow_multapses )
    {
      allow_multapses_ = getValue< bool >( dit->second );
    }
    else if ( dit->first == names::allow_oversized_mask )
    {
      allow_oversized_ = getValue< bool >( dit->second );
    }
    else if ( dit->first == names::number_of_connections )
    {
      number_of_connections = getValue< long >( dit->second );

      if ( number_of_connections < 0 )
      {
        throw BadProperty( "Number of connections cannot be less than zero." );
      }

      number_of_connections_ = number_of_connections;
    }
    else if ( dit->first == names::mask )
    {
      mask_ = NestModule::create_mask( dit->second );
    }
    else if ( dit->first == names::kernel )
    {
      kernel_ = NestModule::create_parameter( dit->second );
    }
    else if ( dit->first == names::synapse_model )
    {
      const std::string syn_name = getValue< std::string >( dit->second );

      const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( syn_name );

      if ( synmodel.empty() )
      {
        throw UnknownSynapseType( syn_name );
      }

      synapse_model_ = { static_cast< index >( synmodel ) };
    }
    else if ( dit->first == names::weight )
    {
      weight_ = { NestModule::create_parameter( dit->second ) };
    }
    else if ( dit->first == names::delay )
    {
      delay_ = { NestModule::create_parameter( dit->second ) };
    }
    else if ( dit->first == names::synapse_parameters )
    {
      ArrayDatum* syn_params_dvd = dynamic_cast< ArrayDatum* >( ( dit->second ).datum() );
      if ( not syn_params_dvd )
      {
        throw BadProperty( "synapse_parameters must be list of dictionaries" );
      }
      for ( auto synapse_datum = syn_params_dvd->begin(); synapse_datum < syn_params_dvd->end(); ++synapse_datum )
      {
        DictionaryDatum* syn_param = dynamic_cast< DictionaryDatum* >( synapse_datum->datum() );

        if ( not( *syn_param )->known( names::synapse_model ) )
        {
          ( **syn_param ).insert( names::synapse_model, "static_synapse" );
        }
        std::string syn_name = ( **syn_param )[ names::synapse_model ];

        if ( not kernel().model_manager.get_synapsedict()->known( syn_name ) )
        {
          throw UnknownSynapseType( syn_name );
        }
        index synapse_model_id = kernel().model_manager.get_synapsedict()->lookup( syn_name );
        synapse_model_.push_back( synapse_model_id );

        DictionaryDatum syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_id );
        if ( ( *syn_param )->known( names::weight ) )
        {
          weight_.push_back( NestModule::create_parameter( ( **syn_param )[ names::weight ] ) );
        }
        else
        {
          weight_.push_back( NestModule::create_parameter( ( *syn_defaults )[ names::weight ] ) );
        }

        if ( ( *syn_param )->known( names::delay ) )
        {
          delay_.push_back( NestModule::create_parameter( ( **syn_param )[ names::delay ] ) );
        }
        else
        {
          if ( not getValue< bool >( ( *syn_defaults )[ names::has_delay ] ) )
          {
            delay_.push_back( NestModule::create_parameter( numerics::nan ) );
          }
          else
          {
            delay_.push_back( NestModule::create_parameter( ( *syn_defaults )[ names::delay ] ) );
          }
        }
      }
    }
    else
    {
      throw BadProperty( "Spatial Connect cannot handle parameter '" + dit->first.toString() + "'." );
    }
  }

  // Set default synapse_model, weight and delay if not given explicitly
  if ( synapse_model_.empty() )
  {
    synapse_model_ = { kernel().model_manager.get_synapsedict()->lookup( "static_synapse" ) };
  }
  DictionaryDatum syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_[ 0 ] );
  if ( weight_.empty() )
  {
    weight_ = { NestModule::create_parameter( ( *syn_defaults )[ names::weight ] ) };
  }
  if ( delay_.empty() )
  {
    if ( not getValue< bool >( ( *syn_defaults )[ names::has_delay ] ) )
    {
      delay_ = { NestModule::create_parameter( numerics::nan ) };
    }
    else
    {
      delay_ = { NestModule::create_parameter( ( *syn_defaults )[ names::delay ] ) };
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

  // Create dummy dictionaries, one per thread
  dummy_param_dicts_.resize( kernel().vp_manager.get_num_threads() );
#pragma omp parallel
  {
    dummy_param_dicts_.at( kernel().vp_manager.get_thread_id() ) = new Dictionary();
  }
}

} // namespace nest
