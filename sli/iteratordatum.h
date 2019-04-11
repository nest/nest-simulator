/*
 *  iteratordatum.h
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

#ifndef ITERATORDATUM_H
#define ITERATORDATUM_H
/*
    Datum template for numeric data types
*/


// this file is based on numericdatum.h and integerdatum.h

// C++ includes:
#include <iostream>

// Includes from libnestutil:
#include "allocator.h"

// Includes from sli:
#include "genericdatum.h"
#include "interpret.h"


// prefixed all references to members of GenericDatum with this->,
// since HP's aCC otherwise complains about them not being declared
// according to ISO Standard Sec. 14.6.2(3) [temp.dep]
// HEP, 2001-08-08

class IteratorState {
public:
  long start;
  long stop;
  long di;
  long pos;

  bool operator==( const IteratorState& i ) const
  {
    return stop == i.stop && start == i.start && di == i.di && pos == i.pos;
  }
};


std::ostream& operator<<( std::ostream&, const IteratorState& );


class IteratorDatum : public GenericDatum< IteratorState, &SLIInterpreter::Iteratortype > {
protected:
  static sli::pool memory;

private:
  Datum*
  clone( void ) const
  {
    return new IteratorDatum( *this );
  }

public:
  // IteratorDatum(start,stop,di));

  IteratorDatum()
  {
    this->d.start = 0;
    this->d.stop = 0;
    this->d.di = 0;
    this->d.pos = 0;
  }
  IteratorDatum( long start_s, long stop_s, long di_s )
  {
    this->d.start = start_s;
    this->d.stop = stop_s;
    this->d.di = di_s;
    this->d.pos = start_s;
  }
  IteratorDatum( const IteratorDatum& d_s )
    : GenericDatum< IteratorState, &SLIInterpreter::Iteratortype >( d_s )
  {
    this->d = d_s.d;
  }
  virtual ~IteratorDatum()
  {
  }


  void
  incr( void )
  {
    this->d.pos += this->d.di;
  }

  void
  decr( void )
  {
    this->d.pos -= this->d.di;
  }

  long
  begin( void )
  {
    return this->d.start;
  }

  long
  end( void )
  {
    return this->d.stop + 1;
  }

  long
  pos( void )
  {
    return this->d.pos;
  }

  long
  size( void )
  {
    return ( this->d.stop - this->d.start ) / this->d.di + 1;
  }

  bool operator==( const IteratorDatum& i ) const
  {
    return this->d == i.d;
  }

  static void* operator new( size_t size )
  {
    if ( size != memory.size_of() ) {
      return ::operator new( size );
    }
    return memory.alloc();
  }

  static void operator delete( void* p, size_t size )
  {
    if ( p == NULL ) {
      return;
    }
    if ( size != memory.size_of() ) {
      ::operator delete( p );
      return;
    }
    memory.free( p );
  }

  void
  print( std::ostream& out ) const
  {
    out << '<' << this->gettypename() << '>';
  }

  void
  pprint( std::ostream& out ) const
  {
    out << this->d;
  }
};


#endif
