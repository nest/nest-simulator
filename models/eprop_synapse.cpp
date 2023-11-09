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

// Includes from nestkernel:
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
  , adam_beta1_( 0.9 )
  , adam_beta2_( 0.999 )
  , adam_epsilon_( 1e-8 )
  , batch_size_( 1 )
  , optimizer_( "gradient_descent" )
  , average_gradient_( false )
{
}

void
EpropCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );
  def< double >( d, names::adam_beta1, adam_beta1_ );
  def< double >( d, names::adam_beta2, adam_beta2_ );
  def< double >( d, names::adam_epsilon, adam_epsilon_ );
  def< long >( d, names::batch_size, batch_size_ );
  def< std::string >( d, names::optimizer, optimizer_ );
  def< bool >( d, names::average_gradient, average_gradient_ );
}

void
EpropCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );
  updateValue< double >( d, names::adam_beta1, adam_beta1_ );
  updateValue< double >( d, names::adam_beta2, adam_beta2_ );
  updateValue< double >( d, names::adam_epsilon, adam_epsilon_ );
  updateValue< long >( d, names::batch_size, batch_size_ );
  updateValue< std::string >( d, names::optimizer, optimizer_ );
  updateValue< bool >( d, names::average_gradient, average_gradient_ );

  if ( adam_beta1_ < 0.0 or 1.0 <= adam_beta1_ )
  {
    throw BadProperty( "adam_beta1 must be in [0,1)" );
  }

  if ( adam_beta2_ < 0.0 or 1.0 <= adam_beta2_ )
  {
    throw BadProperty( "adam_beta2 must be in [0,1)" );
  }

  if ( adam_epsilon_ < 0.0 )
  {
    throw BadProperty( "adam_epsilon must be >= 0" );
  }

  if ( batch_size_ <= 0 )
  {
    throw BadProperty( "batch_size must be > 0" );
  }

  if ( optimizer_ != "gradient_descent" and optimizer_ != "adam" )
  {
    throw BadProperty( "optimizer must be either \"gradient_descent\" or \"adam\"" );
  }
}

}
