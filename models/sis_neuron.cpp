/*
 *  sis_neuron.cpp
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

#include "sis_neuron.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

namespace nest
{

void
register_sis_neuron( const std::string& name )
{
  register_node_model< sis_neuron >( name );
}


sis_update_function::sis_update_function()
  : beta_sis_( 0.1 )
  , mu_sis_( 0.1 )
{
}

void
sis_update_function::get( Dictionary& d ) const
{
  d[ names::beta_sis ] = beta_sis_;
  d[ names::mu_sis ] = mu_sis_;
}

void
sis_update_function::set( const Dictionary& d, Node* node )
{
  update_value_param( d, names::beta_sis, beta_sis_, node );
  if ( beta_sis_ < 0 or beta_sis_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }

  update_value_param( d, names::mu_sis, mu_sis_, node );
  if ( mu_sis_ < 0 or mu_sis_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }
}

template <>
void
RecordablesMap< nest::sis_neuron >::create()
{
  insert_( names::S, &nest::sis_neuron::get_output_state_ );
  insert_( names::h, &nest::sis_neuron::get_input__ );
}

}  // namespace nest
