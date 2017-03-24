/*
 *  tokenstack.h
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

#ifndef TOKENSTACK_H
#define TOKENSTACK_H
/*
    SLI's stack for tokens
*/

// Includes from sli:
#include "tarrayobj.h"
#include "token.h"
#include "tokenarray.h"

/* This stack implementation assumes that functions are only called,
   if the necessary pre-requisites are fulfilled. The code will break
   otherwise.
*/

class TokenStack : private TokenArrayObj
{
public:
  TokenStack( Index n )
    : TokenArrayObj( 0, Token(), n )
  {
  }
  TokenStack( const TokenArray& ta )
    : TokenArrayObj( ta )
  {
  }

  using TokenArrayObj::reserve;
  using TokenArrayObj::reserve_token;

  void
  clear( void )
  {
    erase( begin(), end() );
  }


  void
  push( const Token& e )
  {
    push_back( e );
  }

  void
  push_move( Token& e )
  {
    push_back_move( e );
  }

  /**
   * Push a Token with a valid datum to the stack. This function
   * expects that sufficient space is on the stack to fit the datum.
   * This function increases the reference count of the datum.
   */
  void
  push_by_ref( const Token& e )
  {
    push_back_by_ref( e );
  }

  /**
   * Push a valid datum to the stack. This function expects that
   * sufficient space is on the stack to fit the datum.
   * This function increases the reference count of the datum.
   */
  void
  push_by_pointer( Datum* rhs )
  {
    push_back_by_pointer( rhs );
  }

  void
  pop( void )
  {
    pop_back();
  }

  void pop_move( Token& e ) // new one 5.5.97
  {
    e.move( *( end() - 1 ) );
    pop_back();
  }

  void
  pop( size_t n )
  {
    erase( end() - n, end() );
  }


  Token&
  top( void )
  {
    return *( end() - 1 );
  }
  const Token&
  top( void ) const
  {
    return *( end() - 1 );
  }

  const Token&
  pick( size_t i ) const
  {
    return *( end() - i - 1 );
  }

  Token&
  pick( size_t i )
  {
    return *( end() - i - 1 );
  }

  using TokenArrayObj::empty;
  //  using TokenArray::operator=;


  void
  swap( void )
  {
    ( end() - 1 )->swap( *( end() - 2 ) );
  }

  void
  swap( Token& e )
  {
    ( end() - 1 )->swap( e );
  }

  void
  index( Index i )
  {
    push( pick( i ) );
  }

  void
  roll( size_t n, long k )
  {
    if ( n < 2 || k == 0 )
    {
      return; // nothing to do
    }

    if ( k > 0 )
    {
      rotate( end() - n, end() - ( k % n ), end() );
    }
    else
    {
      rotate( end() - n, end() - ( n + k ) % n, end() );
    }
  }

  Index
  size( void ) const
  {
    return TokenArrayObj::capacity();
  }
  Index
  load( void ) const
  {
    return TokenArrayObj::size();
  }

  void dump( std::ostream& ) const;

  TokenArray
  toArray( void ) const
  {
    return TokenArray( *this );
  }
};

#endif
