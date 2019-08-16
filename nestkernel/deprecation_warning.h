/*
 *  deprecation_warning.h
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

#include <string>

#include "logging_manager.h"

#ifndef DEPRECATION_WARNING_H
#define DEPRECATION_WARNING_H


namespace nest
{

class DeprecationWarning
{
public:
  DeprecationWarning()
    : deprecated_functions_()
  {
  }

  DeprecationWarning( const DeprecationWarning& dw )
    : deprecated_functions_( dw.deprecated_functions_ )
  {
  }

  void set_deprecated( std::string name )
  {
    deprecated_functions_[name] = true;
  }

  void deprecation_warning( std::string name )
  {
    if ( deprecated_functions_[name] )
    {
      LOG( M_DEPRECATED,
        name,
        name + " is deprecated and will be removed in a future version of NEST." );

      deprecated_functions_[name] = false; // to not issue warning again
    }
  }

  void deprecation_warning( std::string name, std::string new_name )
  {
    if ( deprecated_functions_[name] )
    {
      LOG( M_DEPRECATED,
        name,
        name + " is deprecated and will be removed in a future version of NEST, use " + new_name + " instead." );

      deprecated_functions_[name] = false; // to not issue warning again
    }
  }

private:
  std::map<std::string, bool> deprecated_functions_;
};

}

#endif /* DEPRECATION_WARNING_H */
