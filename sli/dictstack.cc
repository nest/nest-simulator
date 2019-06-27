/*
 *  dictstack.cc
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

#include "dictstack.h"

DictionaryStack::DictionaryStack( const Token& t )
  : VoidToken( t )
{
}

DictionaryStack::DictionaryStack( const DictionaryStack& ds )
  : VoidToken( ds.VoidToken )
  , d( ds.d )
{
}

DictionaryStack::~DictionaryStack()
{
  // We have to clear the dictionary before we delete it, otherwise the
  // dictionary references will prevent proper deletion.
  for ( std::list< DictionaryDatum >::iterator i = d.begin(); i != d.end(); ++i )
  {
    ( *i )->clear();
  }
}

void
DictionaryStack::undef( const Name& n )
{

  size_t num_erased = 0;
  for ( std::list< DictionaryDatum >::iterator it = d.begin(); it != d.end(); ++it )
  {
    num_erased += ( *it )->erase( n );
  }
  if ( num_erased == 0 )
  {
    throw UndefinedName( n.toString() );
  }
#ifdef DICTSTACK_CACHE
  clear_token_from_cache( n );
  clear_token_from_basecache( n );
#endif
}

void
DictionaryStack::basedef( const Name& n, const Token& t )
{
//
// insert (n,t) in bottom level dictionary
// dictionary stack must contain at least one dictionary
// VoidToken is an illegal value for t.
#ifdef DICTSTACK_CACHE
  clear_token_from_cache( n );
  basecache_token( n, &( base_->insert( n, t ) ) );
#endif
#ifndef DICTSTACK_CACHE
  ( *base_ )[ n ] = t;
#endif
}


void
DictionaryStack::basedef_move( const Name& n, Token& t )
{
#ifdef DICTSTACK_CACHE
  clear_token_from_cache( n );
  basecache_token( n, &( base_->insert_move( n, t ) ) );
#endif
#ifndef DICTSTACK_CACHE
  base_->insert_move( n, t );
#endif
}


void
DictionaryStack::pop( void )
{
//
// remove top dictionary from stack
// dictionary stack must contain at least one dictionary
//

#ifdef DICTSTACK_CACHE
  clear_dict_from_cache( *( d.begin() ) );
  ( *( d.begin() ) )->remove_dictstack_reference();
#endif
  d.pop_front();
}

void
DictionaryStack::clear( void )
{
  d.erase( d.begin(), d.end() );
#ifdef DICTSTACK_CACHE
  clear_cache();
#endif
}


void
DictionaryStack::top( Token& t ) const
{
  //
  // create a copy of the top level dictionary
  // and move it into Token t.
  // new should throw an exception if it fails
  //

  DictionaryDatum* dd = new DictionaryDatum( *( d.begin() ) );

  Token dt( dd );
  t.move( dt );
}

void
DictionaryStack::toArray( TokenArray& ta ) const
{
  //
  // create a copy of the top level dictionary
  // and move it into Token t.
  // new should throw an exception if it fails
  //

  ta.erase();

  std::list< DictionaryDatum >::const_reverse_iterator i( d.rbegin() );

  while ( i != d.rend() )
  {
    ta.push_back( ( *i ) );
    ++i;
  }
}

void
DictionaryStack::push( Token& d )
{
  DictionaryDatum* dd = dynamic_cast< DictionaryDatum* >( d.datum() );
  assert( dd != NULL );
  push( *dd );
}

void
DictionaryStack::push( const DictionaryDatum& pd )
{
//
// extract Dictionary from Token envelope
// and push it on top of the stack.
// a non dictionary datum at this point is a program bug.
//

#ifdef DICTSTACK_CACHE
  pd->add_dictstack_reference();
  // This call will remove all names in the dict from the name cache.
  clear_dict_from_cache( pd );
#endif

  d.push_front( pd );
}

void
DictionaryStack::set_basedict()
{
  base_ = *( --d.end() ); // Cache base dictionary
}

size_t
DictionaryStack::size( void ) const
{
  //
  // return number of dictionaries on stack
  //

  return d.size();
}


void
DictionaryStack::info( std::ostream& o ) const
{
  //  for_each(d.rbegin(), d.rend(), bind_2nd(mem_fun(&Dictionary::info),o));

  std::list< DictionaryDatum >::const_reverse_iterator i( d.rbegin() );

  o << "DictionaryStack::info" << std::endl;
  o << "Size = " << d.size() << std::endl;
  while ( i != d.rend() )
  {
    ( *i )->info( o );
    ++i;
  }
}

void
DictionaryStack::top_info( std::ostream& o ) const
{
  ( *d.begin() )->info( o );
}

const DictionaryStack& DictionaryStack::operator=( const DictionaryStack& ds )
{
  if ( &ds != this )
  {
    d = ds.d;
#ifdef DICTSTACK_CACHE
    cache_ = ds.cache_;
#endif
  }
  return *this;
}
