/*
 *  deprecation_warning.cpp
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

#ifndef DEPRECATION_WARNING_IMPL_H
#define DEPRECATION_WARNING_IMPL_H

#include "deprecation_warning.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "logging_manager.h"


namespace nest
{

void
DeprecationWarning::deprecation_warning( std::string name )
{
  if ( deprecated_functions_[ name ] )
  {
    LOG( M_DEPRECATED, name, name + " is deprecated and will be removed in a future version of NEST." );

    deprecated_functions_[ name ] = false; // to not issue warning again
  }
}

void
DeprecationWarning::deprecation_warning( std::string name, std::string new_name )
{
  if ( deprecated_functions_[ name ] )
  {
    LOG( M_DEPRECATED,
      name,
      name + " is deprecated and will be removed in a future version of NEST, use " + new_name + " instead." );

    deprecated_functions_[ name ] = false; // to not issue warning again
  }
}
}

#endif /* DEPRECATION_WARNING_IMPL_H */
