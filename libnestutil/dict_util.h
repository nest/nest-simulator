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
#include "nest_datums.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{


/** Update a variable from a dictionary entry if it exists, skip call if it
 * doesn't. If the dictionary entry is a parameter, return value generated from
 * the parameter parameter.
 */
template < typename FT, typename VT >
bool
updateValueParam( DictionaryDatum const& d, Name const n, VT& value, nest::Node* node )
{
  const Token& t = d->lookup( n );

  ParameterDatum* pd = dynamic_cast< ParameterDatum* >( t.datum() );
  if ( pd )
  {
    if ( not node )
    {
      throw BadParameter( "Cannot use Parameter with this model." );
    }
    auto vp = kernel().vp_manager.suggest_vp_for_gid( node->get_gid() );
    auto tid = kernel().vp_manager.vp_to_thread( vp );
    auto rng = get_vp_rng( tid );
    value = pd->get()->value( rng, node );
    return true;
  }
  else
  {
    return updateValue< FT >( d, n, value );
  }
}

} // namespace nest

#endif // DICT_UTIL_H
