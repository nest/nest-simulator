/*
 *  oosupport.h
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

#ifndef OOSUPPORT_H
#define OOSUPPORT_H
/*
    SLI's array access functions
*/

// Includes from sli:
#include "slifunction.h"
#include "slimodule.h"

class OOSupportModule : public SLIModule
{
  class CallMemberFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  CallMemberFunction callmemberfunction;

public:
  OOSupportModule( void )
  {
  }

  void init( SLIInterpreter* );
  const std::string commandstring( void ) const;
  const std::string name( void ) const;
};


#endif
