/*
 *  sirs_neuron.cpp
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

#include "sirs_neuron.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

namespace nest
{

void
register_sirs_neuron( const std::string& name )
{
  register_node_model< sirs_neuron >( name );
}


sirs_update_function::sirs_update_function()
  : beta_sirs_( 0.1 )
  , mu_sirs_( 0.1 )
  , eta_sirs_( 0.1 )
{
}

void
sirs_update_function::get( Dictionary& d ) const
{
  d[ names::beta_sirs ] = beta_sirs_;
  d[ names::mu_sirs ] = mu_sirs_;
  d[ names::eta_sirs ] = eta_sirs_;
}

void
sirs_update_function::set( const Dictionary& d, Node* node )
{
  update_value_param( d, names::beta_sirs, beta_sirs_, node );
  if ( beta_sirs_ < 0 or beta_sirs_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }

  update_value_param( d, names::mu_sirs, mu_sirs_, node );
  if ( mu_sirs_ < 0 or mu_sirs_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }

  update_value_param( d, names::eta_sirs, eta_sirs_, node );
  if ( eta_sirs_ < 0 or eta_sirs_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }
}

template <>
void
RecordablesMap< nest::sirs_neuron >::create()
{
  insert_( names::S, &nest::sirs_neuron::get_output_state_ );
  insert_( names::h, &nest::sirs_neuron::get_input__ );
}

}  // namespace nest
