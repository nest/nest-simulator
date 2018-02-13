/*
 *  numericdatum.h
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

#ifndef NUMERICDATUM_H
#define NUMERICDATUM_H
/*
    Datum template for numeric data types
*/

// Includes from libnestutil:
#include "allocator.h"

// Includes from sli:
#include "genericdatum.h"


// prefixed all references to members of GenericDatum with this->,
// since HP's aCC otherwise complains about them not being declared
// according to ISO Standard Sec. 14.6.2(3) [temp.dep]
// HEP, 2001-08-08

template < class D, SLIType* slt >
class NumericDatum : public GenericDatum< D, slt >
{
protected:
  static sli::pool memory;
  using GenericDatum< D, slt >::d;

private:
  Datum*
  clone( void ) const
  {
    return new NumericDatum< D, slt >( *this );
  }

public:
  NumericDatum()
  {
    d = ( D ) 0;
  }
  NumericDatum( const D& d_s )
  {
    d = d_s;
  }
  virtual ~NumericDatum()
  {
  }

  operator D() const
  {
    return d;
  }

  operator D&()
  {
    return d;
  }

  void input_form( std::ostream& ) const;
  void pprint( std::ostream& ) const;


  static void* operator new( size_t size )
  {
    if ( size != memory.size_of() )
    {
      return ::operator new( size );
    }
    return memory.alloc();
  }

  static void operator delete( void* p, size_t size )
  {
    if ( p == NULL )
    {
      return;
    }
    if ( size != memory.size_of() )
    {
      ::operator delete( p );
      return;
    }
    memory.free( p );
  }
};

#endif
