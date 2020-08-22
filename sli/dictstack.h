/*
 *  dictstack.h
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

#ifndef DICTIONARYSTACK_H
#define DICTIONARYSTACK_H
/*
    SLI's dictionary stack
*/

// C++ includes:
#include <list>
#include <typeinfo>

// Includes from sli:
#include "dictdatum.h"
#include "sliexceptions.h"


/*************************************************************

Problems:

- is it better to uses dictionaries as references to common
  objects like in PS. What is the exact meaning of undef and
  where in our current situation (read RedBook).
- more efficient implementation exploiting
  the name ids (documented elsewhere).

  History:
    (1) using list<Dictionary>
        MD, 23.6.1, Freiburg
    (0) first version (single dictionary)
        MOG, MD, June 1997, Freiburg
**************************************************************/

/**
 * The macro DICTSTACK_CACHE switches on two caches:
 * 1. cache_, global cache for the dictionary stack
 * 2. basecache_, a cache for the system dictionary
 * These caches are direct lookup table with one entry per name.
 * They work as follows:
 * If a name is looked up, it is looked up in the cache.
 * If the cache does not have an entry, the dictionary stack is searched and
 * the name/token combination is added to the cache.
 */
#ifndef DICTSTACK_CACHE
#define DICTSTACK_CACHE 1
#endif

//#undef DICTSTACK_CACHE

class DictionaryStack
{
private:
  const Token VoidToken;
  std::list< DictionaryDatum > d;
  DictionaryDatum base_;
#ifdef DICTSTACK_CACHE
  std::vector< const Token* > cache_;
  std::vector< const Token* > basecache_;
#endif

public:
  DictionaryStack( const Token& = Token() );
  DictionaryStack( const DictionaryStack& );
  ~DictionaryStack();


#ifdef DICTSTACK_CACHE
  /**
   * Add a token to the cache.
   */
  void
  cache_token( const Name& n, const Token* result )
  {
    Name::handle_t key = n.toIndex();
    if ( key >= cache_.size() )
    {
      cache_.resize( Name::num_handles() + 100, 0 );
    }
    cache_[ key ] = result;
  }

  void
  basecache_token( const Name& n, const Token* result )
  {
    Name::handle_t key = n.toIndex();
    if ( key >= basecache_.size() )
    {
      basecache_.resize( Name::num_handles() + 100, 0 );
    }
    basecache_[ key ] = result;
  }

  /**
   * Clear a name from the cache.
   * This function should be called in each def variant.
   */
  void
  clear_token_from_cache( const Name& n )
  {
    Name::handle_t key = n.toIndex();
    if ( key < cache_.size() )
    {
      cache_[ key ] = 0;
    }
  }

  void
  clear_token_from_basecache( const Name& n )
  {
    Name::handle_t key = n.toIndex();
    if ( key < basecache_.size() )
    {
      basecache_[ key ] = 0;
    }
  }

  void
  clear_dict_from_cache( DictionaryDatum d )
  {
    for ( TokenMap::iterator i = d->begin(); i != d->end(); ++i )
    {
      clear_token_from_cache( i->first );
    }
  }


  /** Clear the entire cache.
   * This should be  called whenever a dictionary is pushed or poped.
   * Alternative, one could just clear the names from the moved dictionary.
   */
  void
  clear_cache()
  {
    const size_t cache_size = cache_.size();
    for ( size_t i = 0; i < cache_size; ++i )
    {
      cache_[ i ] = 0;
    }
  }

#endif

  const Token&
  lookup( const Name& n )
  {
    try
    {
      return lookup2( n );
    }
    catch ( UndefinedName& )
    {
      return VoidToken;
    }
  }

  const Token&
  lookup2( const Name& n )
  {
#ifdef DICTSTACK_CACHE
    Name::handle_t key = n.toIndex();
    if ( key < cache_.size() )
    {
      const Token* result = cache_[ key ];
      if ( result )
      {
        return *result;
      }
    }
#endif

    std::list< DictionaryDatum >::const_iterator i = d.begin();

    while ( i != d.end() )
    {
      TokenMap::const_iterator where = ( *i )->find( n );
      if ( where != ( *i )->end() )
      {
#ifdef DICTSTACK_CACHE
        cache_token( n, &( where->second ) ); // Update the cache
#endif
        return where->second;
      }
      ++i;
    }
    throw UndefinedName( n.toString() );
  }

  /** Lookup a name searching only the bottom level dictionary.
   *  If the Name is not found,
   *  @a VoidToken is returned.
   */
  const Token& baselookup( const Name& n ) // lookup in a specified
  {                                        // base dictionary
#ifdef DICTSTACK_CACHE
    Name::handle_t key = n.toIndex();
    if ( key < basecache_.size() )
    {
      const Token* result = basecache_[ key ];
      if ( result )
      {
        return *result;
      }
    }
#endif
    TokenMap::const_iterator where = base_->find( n );

    if ( where != base_->end() )
    {
#ifdef DICTSTACK_CACHE
      cache_token( n, &( where->second ) );     // Update the cache
      basecache_token( n, &( where->second ) ); // and the basecache
#endif
      return where->second;
    }
    else
    {
      return VoidToken;
    }
  }

  /** Test for a name searching all dictionaries on the stack.
   */
  bool
  known( const Name& n )
  {
#ifdef DICTSTACK_CACHE
    Name::handle_t key = n.toIndex();
    if ( key < cache_.size() )
    {
      const Token* result = cache_[ key ];
      if ( result )
      {
        return true;
      }
    }
#endif
    std::list< DictionaryDatum >::const_iterator i( d.begin() );

    while ( i != d.end() )
    {
      TokenMap::const_iterator where = ( *i )->find( n );
      if ( where != ( *i )->end() )
      {
#ifdef DICTSTACK_CACHE
        cache_token( n, &( where->second ) ); // Update the cache
#endif
        return true;
      }
      ++i;
    }
    return false;
  }

  /** Test for a name in the bottom level dictionary.
   */
  bool baseknown( const Name& n ) // lookup in a specified
  {                               // base dictionary
#ifdef DICTSTACK_CACHE
    Name::handle_t key = n.toIndex();
    if ( key < basecache_.size() )
    {
      const Token* result = basecache_[ key ];
      if ( result )
      {
        return true;
      }
    }
#endif
    TokenMap::const_iterator where = base_->find( n );
    if ( where != base_->end() )
    {
#ifdef DICTSTACK_CACHE
      basecache_token( n, &( where->second ) ); // Update the basecache
      cache_token( n, &( where->second ) );     // and the cache
#endif
      return true;
    }
    return false;
  }


  //
  // def and def_move operate on the top level dictionary.
  // undef is not defined for the dictionary stack.

  /** Bind a Token to a Name in the top level dictionary.
   *  The Token is copied.
   */
  void def( const Name&, const Token& );

  /** Unbind a previously defined Name from its token. Seach in all
   * dictionaries.
   */
  void undef( const Name& );

  /** Bind a Token to a Name in the bottom level dictionary.
   *  The Token is copied.
   */
  void basedef( const Name& n, const Token& t );

  /** Bind a Token to a Name in the top level dictionary.
   *  The Token is moved.
   */
  void def_move( const Name&, Token& );

  /**
   * This function must be called once to initialize the systemdict cache.
   */
  void set_basedict();

  /** Bind a Token to a Name in the bottom level dictionary.
   *  The Token is moved.
   */
  void basedef_move( const Name& n, Token& t );

  bool where( const Name&, Token& );

  void pop( void );


  //
  // a Dictionary is always wrapped in a Token
  //
  void top( Token& ) const;
  void push( const DictionaryDatum& );
  void push( Token& );

  void clear( void );
  void toArray( TokenArray& ) const;
  //
  // move is efficient for interaction with operand and execution
  // stack, but can not be implemented in this version
  //
  //    void pop_move(Token &) const;
  //    void push_move(const Token &);

  //
  // number of dictionaries currently on the stack
  //
  size_t size( void ) const;


  //
  // info for debugging purposes.
  // calls info(ostream&) for all dictionaries
  //
  void info( std::ostream& ) const;
  void top_info( std::ostream& ) const; // calls info of top dictionary
  const DictionaryStack& operator=( const DictionaryStack& );
};

inline void
DictionaryStack::def( const Name& n, const Token& t )
{
//
// insert (n,t) in top level dictionary
// dictionary stack must contain at least one dictionary
// VoidToken is an illegal value for t.
//
#ifdef DICTSTACK_CACHE
  cache_token( n, &( ( *d.begin() )->insert( n, t ) ) );
#endif
#ifndef DICTSTACK_CACHE
  ( **d.begin() )[ n ] = t;
#endif
}

inline void
DictionaryStack::def_move( const Name& n, Token& t )
{
//
// insert (n,t) in top level dictionary
// dictionary stack must contain at least one dictionary
// VoidToken is an illegal value for t.
// def_move returns t as the VoidToken.
//
/* clear_token_from_cache(n); */

#ifdef DICTSTACK_CACHE
  cache_token( n, &( ( *d.begin() )->insert_move( n, t ) ) );
#endif
#ifndef DICTSTACK_CACHE
  ( *d.begin() )->insert_move( n, t );
#endif
}

#endif
