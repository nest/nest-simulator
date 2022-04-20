/*
 *  triedatum.h
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

#ifndef TYPEDFUNCTIONDATUM_H
#define TYPEDFUNCTIONDATUM_H

// C++ includes:
#include <typeinfo>

// Includes from libnestutil:
#include "allocator.h"

// Includes from sli:
#include "datum.h"
#include "interpret.h"
#include "slifunction.h"
#include "typechk.h"

class TrieDatum : public TypedDatum< &SLIInterpreter::Trietype >
{
protected:
  static sli::pool memory;

private:
  Name name;
  TypeTrie tree;

  Datum*
  clone( void ) const
  {
    return new TrieDatum( *this );
  }

  Datum*
  get_ptr()
  {
    Datum::addReference();
    return this;
  }

public:
  TrieDatum( TrieDatum const& fd )
    : TypedDatum< &SLIInterpreter::Trietype >( fd )
    , name( fd.name )
    , tree( fd.tree )
  {
    set_executable();
  }

  TrieDatum( Name const& n )
    : name( n )
    , tree()
  {
    set_executable();
  }

  TrieDatum( Name const& n, const TokenArray& ta )
    : name( n )
    , tree( ta )
  {
    set_executable();
  }


  void
  print( std::ostream& o ) const
  {
    o << '+' << name << '+';
  }

  void
  pprint( std::ostream& o ) const
  {
    print( o );
  }

  void
  info( std::ostream& out ) const
  {
    pprint( out );
    out << "\nVariants are:" << std::endl;
    tree.info( out );
  }

  bool equals( Datum const* ) const;

  const Name&
  getname( void ) const
  {
    return name;
  }

  void
  insert( const TypeArray& a, const Token& t )
  {
    tree.insert( a, t );
  }

  void
  insert_move( const TypeArray& a, Token& t )
  {
    tree.insert_move( a, t );
  }

  const Token&
  lookup( const TokenStack& s ) const
  {
    return tree.lookup( s );
  }

  TypeTrie&
  get( void )
  {
    return tree;
  }

  static void*
  operator new( size_t size )
  {
    if ( size != memory.size_of() )
    {
      return ::operator new( size );
    }
    return memory.alloc();
  }

  static void
  operator delete( void* p, size_t size )
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
