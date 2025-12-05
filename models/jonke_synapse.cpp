/*
 *  jonke_synapse.cpp
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

#include "jonke_synapse.h"

#include "nest_impl.h"

void
nest::register_jonke_synapse( const std::string& name )
{
  register_connection_model< jonke_synapse >( name );
}

namespace nest
{

JonkeCommonProperties::JonkeCommonProperties()
  : CommonSynapseProperties()
  , alpha_( 1.0 )
  , beta_( 0.0 )
  , lambda_( 0.01 )
  , mu_plus_( 0.0 )
  , mu_minus_( 0.0 )
  , tau_plus_( 20.0 )
  , Wmax_( 100.0 )
{
}

void
JonkeCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  def< double >( d, names::alpha, alpha_ );
  def< double >( d, names::beta, beta_ );
  def< double >( d, names::lambda, lambda_ );
  def< double >( d, names::mu_plus, mu_plus_ );
  def< double >( d, names::mu_minus, mu_minus_ );
  def< double >( d, names::tau_plus, tau_plus_ );
  def< double >( d, names::Wmax, Wmax_ );
}

void
JonkeCommonProperties::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  updateValue< double >( d, names::alpha, alpha_ );
  updateValue< double >( d, names::beta, beta_ );
  updateValue< double >( d, names::lambda, lambda_ );
  updateValue< double >( d, names::tau_plus, tau_plus_ );
  updateValue< double >( d, names::mu_plus, mu_plus_ );
  updateValue< double >( d, names::mu_minus, mu_minus_ );
  updateValue< double >( d, names::Wmax, Wmax_ );
}


}
