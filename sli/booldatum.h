/*
 *  booldatum.h
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

#ifndef BOOLDATUM_H
#define BOOLDATUM_H

// C++ includes:
#include <string>

// Includes from libnestutil:
#include "allocator.h"

// Includes from sli:
#include "genericdatum.h"
#include "interpret.h"


class Name;
class Token;

class BoolDatum : public GenericDatum< bool, &SLIInterpreter::Booltype >
{
protected:
  static sli::pool memory;

private:
  Datum*
  clone( void ) const
  {
    return new BoolDatum( *this );
  }

public:
  static const char* true_string;
  static const char* false_string;

  BoolDatum()
    : GenericDatum< bool, &SLIInterpreter::Booltype >()
  {
  }
  BoolDatum( const BoolDatum& val )
    : GenericDatum< bool, &SLIInterpreter::Booltype >( val )
  {
  }
  BoolDatum( bool val )
    : GenericDatum< bool, &SLIInterpreter::Booltype >( val )
  {
  }
  BoolDatum( const Name& );

  operator bool() const
  {
    return d;
  }

  operator Name() const;

  operator std::string() const;

  void input_form( std::ostream& ) const;
  void print( std::ostream& ) const;
  void pprint( std::ostream& ) const;

  static void* operator new( size_t size );

  static void operator delete( void* p, size_t size );
};

#endif
