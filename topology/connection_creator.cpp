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
  , source_filter_()
  , target_filter_()
  , number_of_connections_()
  , mask_()
  , kernel_()
  , synapse_model_( kernel().model_manager.get_synapsedict()->lookup(
      "static_synapse" ) )
  , weight_()
  , delay_()
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

      mask_ = TopologyModule::create_mask( dit->second );
    }
    else if ( dit->first == names::kernel )
    {

      kernel_ = TopologyModule::create_parameter( dit->second );
    }
    else if ( dit->first == names::synapse_model )
    {

      const std::string syn_name = getValue< std::string >( dit->second );

      const Token synmodel =
        kernel().model_manager.get_synapsedict()->lookup( syn_name );

      if ( synmodel.empty() )
      {
        throw UnknownSynapseType( syn_name );
      }

      synapse_model_ = static_cast< index >( synmodel );
    }
    else if ( dit->first == names::targets )
    {

      target_filter_ = getValue< DictionaryDatum >( dit->second );
    }
    else if ( dit->first == names::sources )
    {

      source_filter_ = getValue< DictionaryDatum >( dit->second );
    }
    else if ( dit->first == names::weights )
    {

      weight_ = TopologyModule::create_parameter( dit->second );
    }
    else if ( dit->first == names::delays )
    {

      delay_ = TopologyModule::create_parameter( dit->second );
    }
    else
    {

      throw BadProperty( "ConnectLayers cannot handle parameter '"
        + dit->first.toString() + "'." );
    }
  }

  // Set default weight and delay if not given explicitly
  DictionaryDatum syn_defaults =
    kernel().model_manager.get_connector_defaults( synapse_model_ );
  if ( not weight_.valid() )
  {
    weight_ =
      TopologyModule::create_parameter( ( *syn_defaults )[ names::weight ] );
  }
  if ( not delay_.valid() )
  {
    if ( not getValue< bool >( ( *syn_defaults )[ names::has_delay ] ) )
    {
      delay_ = TopologyModule::create_parameter( numerics::nan );
    }
    else
    {
      delay_ =
        TopologyModule::create_parameter( ( *syn_defaults )[ names::delay ] );
    }
  }

  if ( connection_type == names::convergent )
  {

    if ( number_of_connections >= 0 )
    {
      type_ = Convergent;
    }
    else
    {
      type_ = Target_driven;
    }
  }
  else if ( connection_type == names::divergent )
  {

    if ( number_of_connections >= 0 )
    {
      type_ = Divergent;
    }
    else
    {
      type_ = Source_driven;
    }
  }
  else
  {

    throw BadProperty( "Unknown connection type." );
  }
}

} // namespace nest
