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

// nestkernel
#include "nest_impl.h"


namespace nest
{

void
register_eprop_synapse( const std::string& name )
{
  register_connection_model< eprop_synapse >( name );
}

EpropCommonProperties::EpropCommonProperties()
  : CommonSynapseProperties()
  , average_gradient_( false )
  , optimizer_cp_( new EpropOptimizerCommonPropertiesGradientDescent() )
{
}

EpropCommonProperties::EpropCommonProperties( const EpropCommonProperties& cp )
  : CommonSynapseProperties( cp )
  , average_gradient_( cp.average_gradient_ )
  , optimizer_cp_( cp.optimizer_cp_->clone() )
{
}

EpropCommonProperties::~EpropCommonProperties()
{
  delete optimizer_cp_;
}

void
EpropCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );
  def< bool >( d, names::average_gradient, average_gradient_ );
  def< std::string >( d, names::optimizer, optimizer_cp_->get_name() );
  DictionaryDatum optimizer_status = new Dictionary;
  optimizer_cp_->get_status( optimizer_status );
  ( *d )[ names::optimizer_status ] = optimizer_status;
}

template <>
void
GenericConnectorModel< eprop_synapse< TargetIdentifierPtrRport > >::set_optimizer_on_default_connection(
  EpropOptimizer* const optimizer )
{
  default_connection_.optimizer_ = optimizer;
}

template <>
void
GenericConnectorModel< eprop_synapse< TargetIdentifierIndex > >::set_optimizer_on_default_connection(
  EpropOptimizer* const optimizer )
{
  default_connection_.optimizer_ = optimizer;
}

void
EpropCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );
  updateValue< bool >( d, names::average_gradient, average_gradient_ );

  std::string new_optimizer;
  const bool set_optimizer = updateValue< std::string >( d, names::optimizer, new_optimizer );
  if ( set_optimizer and new_optimizer != optimizer_cp_->get_name() )
  {
    if ( kernel().connection_manager.get_num_connections( cm.get_syn_id() ) > 0 )
    {
      throw BadParameter( "The optimizer cannot be changed because synapses have been created." );
    }

    // TODO: Selection here should be based on an optimizer registry and a factory.
    // delete is in if/elif because we must delete only when we are sure that we have a valid optimizer.
    if ( new_optimizer == "gradient_descent" )
    {
      delete optimizer_cp_;
      optimizer_cp_ = new EpropOptimizerCommonPropertiesGradientDescent();
    }
    else if ( new_optimizer == "adam" )
    {
      delete optimizer_cp_;
      optimizer_cp_ = new EpropOptimizerCommonPropertiesAdam();
    }
    else
    {
      throw BadProperty( "optimizer must be chosen from [\"gradient_descent\", \"adam\"]" );
    }

    cm.set_optimizer_on_default_connection( optimizer_cp_->get_optimizer() );
  }

  // We can now set the defaults on the new optimizer common props
  // optimizer_cp_->set_status( d[ names::optimizer_status ] );
}

} // namespace nest
