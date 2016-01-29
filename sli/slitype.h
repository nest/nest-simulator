/*
 *  slitype.h
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

#ifndef SLITYPE_H
#define SLITYPE_H
/*
    class implementing SLI types
*/

// C++ includes:
#include <iostream>
#include <typeinfo>

// Includes from sli:
#include "name.h"
#include "slifunction.h"


/* class SLIType represents the SLI type information. Each
   Datum object carries a pointer to the SLIType object, representing the
   SLI type of its contents.
   class SLIType is such that there can be only one instance for each
   typename.
   The typename is assigned at a special position with local scope, e.g.
   the constructor of a client class. This reduces the risk of running into
   the static initialization problem, since class Name has static members
   which might not be constructet when SLITypes are used as static
   data members.
*/

class SLIInterpreter;

class SLIType
{
  unsigned int count;

  Name* name;
  SLIFunction* defaultaction;

  SLIType& operator=( const SLIType& );
  SLIType( const SLIType& );

public:
  SLIType( void )
    : count( 0 )
    , name( NULL )
    , defaultaction( NULL )
  {
  }
  ~SLIType()
  {
  }

  void settypename( const std::string& s );
  void deletetypename( void );

  const Name&
  gettypename( void ) const
  {
    //      assert( name !=NULL);
    return *name;
  }

  void setdefaultaction( SLIFunction& );


  SLIFunction*
  getaction( void ) const
  {
    // assert(defaultaction != NULL);
    // If this fails, we have created a datum before the types were initialised.
    return defaultaction;
  }
};

// This shows how the type-objects are to be used
// class SLIInterpreter
// {

// public:
//     static const char* Integertypename;
//     static const char* Doubletypename;

//     static SLIType Integertype;
//     static SLIType Doubletype;

//     Interpreter(void);
// };

// Interpreter::Interpreter(void)
// {
//     Integertype.settypename(Integertypename);
//     Doubletype.settypename(Doubletypename);
// }

#endif
