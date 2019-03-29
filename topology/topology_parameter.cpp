/*
 *  topology_parameter.cpp
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

#include "topology_parameter.h"

// includes from sli
#include "lockptrdatum_impl.h"

// Explicit definition required to ensure visibility when compiling with
// clang under OSX. This must be outside namespace NEST, since the template
// is defined in the global namespace.
template class lockPTRDatum< nest::TopologyParameter,
  &nest::TopologyModule::TopologyParameterType >;

namespace nest
{

double
TopologyParameter::value( const std::vector< double >& pt,
  librandom::RngPtr& rng ) const
{
  switch ( pt.size() )
  {
  case 2:
    return value( Position< 2 >( pt ), rng );
  case 3:
    return value( Position< 3 >( pt ), rng );
  default:
    throw BadProperty( "Position must be 2- or 3-dimensional." );
  }
}

Gaussian2DTopologyParameter::Gaussian2DTopologyParameter( const DictionaryDatum& d )
  : c_( 0.0 )
  , p_center_( 1.0 )
  , mean_x_( 0.0 )
  , sigma_x_( 1.0 )
  , mean_y_( 0.0 )
  , sigma_y_( 1.0 )
  , rho_( 0.0 )
{
  updateValue< double >( d, names::c, c_ );
  updateValue< double >( d, names::p_center, p_center_ );
  updateValue< double >( d, names::mean_x, mean_x_ );
  updateValue< double >( d, names::sigma_x, sigma_x_ );
  updateValue< double >( d, names::mean_y, mean_y_ );
  updateValue< double >( d, names::sigma_y, sigma_y_ );
  updateValue< double >( d, names::rho, rho_ );
  if ( rho_ >= 1 || rho_ <= -1 )
  {
    throw BadProperty(
      "topology::Gaussian2DParameter: "
      "-1 < rho < 1 required." );
  }
  if ( sigma_x_ <= 0 || sigma_y_ <= 0 )
  {
    throw BadProperty(
      "topology::Gaussian2DParameter: "
      "sigma_x > 0 and sigma_y > 0 required." );
  }
}

} // namespace nest
