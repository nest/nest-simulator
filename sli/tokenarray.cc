/*
 *  tokenarray.cc
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

#include "tokenarray.h"

// Includes from sli:
#include "doubledatum.h"
#include "integerdatum.h"
#include "stringdatum.h"
#include "tokenutils.h"


const TokenArray& TokenArray::operator=( const TokenArray& a )
{
  a.data->add_reference(); // protect from a=a
  data->remove_reference();
  data = a.data;

  return *this;
}


TokenArray::TokenArray( const std::vector< long >& a )
  : data( new TokenArrayObj( a.size(), Token(), 0 ) )
{
  assert( data != NULL );
  for ( size_t i = 0; i < a.size(); ++i )
  {
    Token idt( new IntegerDatum( a[ i ] ) );
    ( *data )[ i ].move( idt );
  }
}

TokenArray::TokenArray( const std::vector< size_t >& a )
  : data( new TokenArrayObj( a.size(), Token(), 0 ) )
{
  assert( data != NULL );
  for ( size_t i = 0; i < a.size(); ++i )
  {
    Token idt( new IntegerDatum( a[ i ] ) );
    ( *data )[ i ].move( idt );
  }
}

TokenArray::TokenArray( const std::vector< double >& a )
  : data( new TokenArrayObj( a.size(), Token(), 0 ) )
{
  assert( data != NULL );
  for ( size_t i = 0; i < a.size(); ++i )
  {
    Token ddt( new DoubleDatum( a[ i ] ) );
    ( *data )[ i ].move( ddt );
  }
}

TokenArray::TokenArray( const std::vector< float >& a )
  : data( new TokenArrayObj( a.size(), Token(), 0 ) )
{
  assert( data != NULL );
  for ( size_t i = 0; i < a.size(); ++i )
  {
    Token ddt( new DoubleDatum( a[ i ] ) );
    ( *data )[ i ].move( ddt );
  }
}


void
TokenArray::toVector( std::vector< long >& a ) const
{
  a.clear();
  a.reserve( size() );
  for ( Token* idx = begin(); idx != end(); ++idx )
  {
    IntegerDatum* targetid = dynamic_cast< IntegerDatum* >( idx->datum() );
    if ( targetid == NULL )
    {
      IntegerDatum const d;
      throw TypeMismatch( d.gettypename().toString(), idx->datum()->gettypename().toString() );
    }

    a.push_back( targetid->get() );
  }
}

void
TokenArray::toVector( std::vector< size_t >& a ) const
{
  a.clear();
  a.reserve( size() );
  for ( Token* idx = begin(); idx != end(); ++idx )
  {
    IntegerDatum* targetid = dynamic_cast< IntegerDatum* >( idx->datum() );
    if ( targetid == NULL )
    {
      IntegerDatum const d;
      throw TypeMismatch( d.gettypename().toString(), idx->datum()->gettypename().toString() );
    }

    a.push_back( targetid->get() );
  }
}

void
TokenArray::toVector( std::vector< double >& a ) const
{
  a.clear();
  a.reserve( size() );
  for ( Token* idx = begin(); idx != end(); ++idx )
  {
    DoubleDatum* targetid = dynamic_cast< DoubleDatum* >( idx->datum() );
    if ( targetid == NULL )
    {
      DoubleDatum const d;
      throw TypeMismatch( d.gettypename().toString(), idx->datum()->gettypename().toString() );
    }
    a.push_back( targetid->get() );
  }
}

void
TokenArray::toVector( std::vector< std::string >& a ) const
{
  a.clear();
  a.reserve( size() );
  for ( Token* idx = begin(); idx != end(); ++idx )
  {
    std::string* target = dynamic_cast< std::string* >( idx->datum() );
    if ( target == NULL )
    {
      StringDatum const d;
      throw TypeMismatch( d.gettypename().toString(), idx->datum()->gettypename().toString() );
    }
    a.push_back( *target );
  }
}


bool
TokenArray::valid( void ) const
{
  if ( data == NULL )
  {
    return false;
  }
  return data->valid();
}


std::ostream& operator<<( std::ostream& out, const TokenArray& a )
{

  for ( Token* t = a.begin(); t < a.end(); ++t )
  {
    out << *t << ' ';
  }

  return out;
}
