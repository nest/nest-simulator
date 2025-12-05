/*
 *  eprop_synapse.cpp
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

#include "eprop_synapse.h"

#include "connection_manager.h"
#include "nest_impl.h"

// nestkernel

namespace nest
{

void
register_eprop_synapse( const std::string& name )
{
  register_connection_model< eprop_synapse >( name );
}

EpropSynapseCommonProperties::EpropSynapseCommonProperties()
  : CommonSynapseProperties()
  , optimizer_cp_( new WeightOptimizerCommonPropertiesGradientDescent() )
{
}

EpropSynapseCommonProperties::EpropSynapseCommonProperties( const EpropSynapseCommonProperties& cp )
  : CommonSynapseProperties( cp )
  , optimizer_cp_( cp.optimizer_cp_->clone() )
{
}

EpropSynapseCommonProperties::~EpropSynapseCommonProperties()
{
  delete optimizer_cp_;
}

void
EpropSynapseCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );
  def< std::string >( d, names::optimizer, optimizer_cp_->get_name() );
  DictionaryDatum optimizer_dict = new Dictionary;
  optimizer_cp_->get_status( optimizer_dict );
  ( *d )[ names::optimizer ] = optimizer_dict;
}

void
EpropSynapseCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  if ( d->known( names::optimizer ) )
  {
    DictionaryDatum optimizer_dict = getValue< DictionaryDatum >( d->lookup( names::optimizer ) );

    std::string new_optimizer;
    const bool set_optimizer = updateValue< std::string >( optimizer_dict, names::type, new_optimizer );
    if ( set_optimizer and new_optimizer != optimizer_cp_->get_name() )
    {
      if ( kernel::manager< ConnectionManager >.get_num_connections( cm.get_syn_id() ) > 0 )
      {
        throw BadParameter( "The optimizer cannot be changed because synapses have been created." );
      }

      // TODO: selection here should be based on an optimizer registry and a factory
      // delete is in if/else if because we must delete only when we are sure that we have a valid optimizer
      if ( new_optimizer == "gradient_descent" )
      {
        delete optimizer_cp_;
        optimizer_cp_ = new WeightOptimizerCommonPropertiesGradientDescent();
      }
      else if ( new_optimizer == "adam" )
      {
        delete optimizer_cp_;
        optimizer_cp_ = new WeightOptimizerCommonPropertiesAdam();
      }
      else
      {
        throw BadProperty( "optimizer from [\"gradient_descent\", \"adam\"] required." );
      }
    }

    // we can now set the defaults on the new optimizer common properties
    optimizer_cp_->set_status( optimizer_dict );
  }
}

template <>
void
Connector< eprop_synapse< TargetIdentifierPtrRport > >::disable_connection( const size_t lcid )
{
  assert( not C_[ lcid ].is_disabled() );
  C_[ lcid ].disable();
}

template <>
void
Connector< eprop_synapse< TargetIdentifierIndex > >::disable_connection( const size_t lcid )
{
  assert( not C_[ lcid ].is_disabled() );
  C_[ lcid ].disable();
}


template <>
Connector< eprop_synapse< TargetIdentifierPtrRport > >::~Connector()
{
  for ( auto& c : C_ )
  {
    c.delete_optimizer();
  }
  C_.clear();
}

template <>
Connector< eprop_synapse< TargetIdentifierIndex > >::~Connector()
{
  for ( auto& c : C_ )
  {
    if ( not c.is_disabled() )
    {
      c.delete_optimizer();
    }
  }
  C_.clear();
}


} // namespace nest
