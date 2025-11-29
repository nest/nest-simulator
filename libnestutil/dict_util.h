/*
 *  dict_util.h
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

#ifndef DICT_UTIL_H
#define DICT_UTIL_H

// Includes from nestkernel:
#include "kernel_manager.h"
#include "vp_manager_impl.h"

#include "dictionary.h"

namespace nest
{
/**
 * Obtain value from parameter dictionary including evaluation of random or spatial parameters.
 *
 * This function should be used instead of `update_value()` everywhere where the user may pass
 * random or spatial parameters.
 */
template < typename T >
bool
update_value_param( dictionary const& d, const std::string& key, T& value, nest::Node* node )
{
  assert( node != nullptr ); // PYNEST-NG-FUTURE: Receive node as const Node&, but that needs many changes throughout
  const auto it = d.find( key );
  if ( it != d.end() and is_type< std::shared_ptr< nest::Parameter > >( it->second.item ) )
  {
    const auto param = d.get< ParameterPTR >( key );
    const auto vp = kernel().vp_manager.node_id_to_vp( node->get_node_id() );
    const auto tid = kernel().vp_manager.vp_to_thread( vp );
    const auto rng = get_vp_specific_rng( tid );
    value = param->value( rng, node );
    return true;
  }
  else
  {
    return d.update_value( key, value );
  }
}

} // namespace nest

#endif // DICT_UTIL_H
