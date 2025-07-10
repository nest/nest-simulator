/*
 *  gauss_rate.cpp
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

#include "gauss_rate.h"

// Includes from nestkernel
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "nest_impl.h"

namespace nest
{
void
register_gauss_rate_ipn( const std::string& name )
{
  register_node_model< gauss_rate_ipn >( name );
}

void
register_rate_transformer_gauss( const std::string& name )
{
  register_node_model< rate_transformer_gauss >( name );
}


void
nonlinearities_gauss_rate::get( dictionary& d ) const
{
  d[ names::g ] = g_;
  d[ names::mu ] = mu_;
  d[ names::sigma ] = sigma_;
}

void
nonlinearities_gauss_rate::set( const dictionary& d, Node* node )
{
  update_value_param( d, names::g, g_, node );
  update_value_param( d, names::mu, mu_, node );
  update_value_param( d, names::sigma, sigma_, node );
}

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< nest::gauss_rate_ipn >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::rate, &nest::gauss_rate_ipn::get_rate_ );
  insert_( names::noise, &nest::gauss_rate_ipn::get_noise_ );
}

template <>
void
RecordablesMap< nest::rate_transformer_gauss >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::rate, &nest::rate_transformer_gauss::get_rate_ );
}

} // namespace nest
