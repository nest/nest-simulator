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

// C++ includes:
#include <string>
#include <map>
#include <iostream>

#ifndef DEPRECATION_WARNING_H
#define DEPRECATION_WARNING_H


/** @BeginDocumentation

   Name: DeprecationWarning - General deprecation warning class for models with
                              deprecated parameters

   Description:
   General class for handling deprecations. The deprecation warning will only
   be issued the first time the deprecated parameter is updated.

   How to use:
   In model constructor put
     this->deprecation_warning.set_deprecated(deprecated_parameter);

   In function updating the deprecated parameter put
     node->deprecation_warning.deprecation_warning(deprecated_parameter);
   or
     node->deprecation_warning.deprecation_warning(deprecated_parameter,
                                                   new_parameter);
 */


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

  /*
   * Set parameter name to be deprecated.
   */
  void
  set_deprecated( std::string name )
  {
    deprecated_functions_[ name ] = true;
  }

  /*
   * Issues deprecation warning.
   */
  void deprecation_warning( std::string name );
  void deprecation_warning( std::string name, std::string new_name );

private:
  std::map< std::string, bool > deprecated_functions_;
};
}

#endif /* DEPRECATION_WARNING_H */
