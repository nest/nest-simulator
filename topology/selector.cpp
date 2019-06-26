/*
 *  selector.cpp
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

#include "selector.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

// Includes from topology:
#include "topology_names.h"


namespace nest
{

Selector::Selector( const DictionaryDatum& d )
  : model( -1 )
  , depth( -1 )
{
  if ( updateValue< long >( d, names::lid, depth ) )
  {
    if ( depth <= 0 )
    {
      throw BadProperty( "lid must be >0" );
    }

    depth -= 1; // lid starts at 1 for backwards compatibility
  }

  std::string modelname;
  if ( updateValue< std::string >( d, names::model, modelname ) )
  {

    const Token model_token = kernel().model_manager.get_modeldict()->lookup( modelname );

    if ( model_token.empty() )
    {
      throw UnknownModelName( modelname );
    }
    model = static_cast< long >( model_token );
  }
}

} // namespace nest
