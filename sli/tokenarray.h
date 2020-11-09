/*
 *  tokenarray.h
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

#ifndef TOKENARRAY_H
#define TOKENARRAY_H
/*
    Reference Counted Array class specialized on Token objects.
*/

// C++ includes:
#include <algorithm>
#include <cstddef>
#include <typeinfo>
#include <vector>

// Includes from sli:
#include "sliexceptions.h"
#include "tarrayobj.h"

// class Tokenarray uses reference counting and lazy evaluation.
// This means that only a pointer
// to the actual array representation, class TokenArrayObj, is stored.
// In most cases only this pointer is copied. Only when the array is
// to be modified, will the TokenArray be copied.

// class TokenArrayObj is the complete token array class without reference-
// counting and lazu evaluation. class TokenArray serves as a smart interface.
//
// Note, that after the use of non const member functions, iterators (i.e.
// Token *) may become invalid due to cloning or reallocation of the array
//
//
// gewaltig, July 18 1997


// _move member functions may invalidate their iterator arguments.
// These functions are most efficient if for the source (argument)
// TokenArray references()==1 holds and therfore no cloning has to
// be performed.
//
// can be optimized for the case references()>1 by only
// constructing the elements needed without proper clone()
//
// Diesmann, April 14 1998

class TokenArray
{
private:
  TokenArrayObj* data;

  bool
  clone( void )
  {
    if ( data->references() > 1 )
    {
      data->remove_reference();
      data = new TokenArrayObj( *data );
      return true;
    }
    else
    {
      return false;
    }
  }

  bool
  detach( void )
  {
    if ( data->references() > 1 )
    {
      data->remove_reference();
      data = new TokenArrayObj();
      return true;
    }
    else
    {
      return false;
    }
  }

protected:
  friend class TokenArrayObj;
  friend class TokenStack;
  operator TokenArrayObj() const
  {
    return *data;
  }

public:
  TokenArray( void )
    : data( new TokenArrayObj() ){};

  explicit TokenArray( size_t n, const Token& t = Token(), size_t alloc = 128 )
    : data( new TokenArrayObj( n, t, alloc ) )
  {
  }

  TokenArray( const TokenArray& a )
    : data( a.data )
  {
    data->add_reference();
  }

  TokenArray( const TokenArrayObj& a )
    : data( new TokenArrayObj( a ) )
  {
  }

  TokenArray( const std::vector< size_t >& );
  TokenArray( const std::vector< long >& );
  TokenArray( const std::vector< double >& );
  TokenArray( const std::vector< float >& );

  virtual ~TokenArray()
  {
    data->remove_reference(); // this will dispose data if needed.
  }

  /**
   * Return pointer to the first element.
   */
  Token*
  begin() const
  {
    return data->begin();
  }

  /**
   * Return pointer to next to last element.
   */
  Token*
  end() const
  {
    return data->end();
  }

  /**
   * Return number of elements in the array.
   */
  size_t
  size( void ) const
  {
    return data->size();
  }

  /**
   * Return maximal number of elements that fit into the container.
   */
  size_t
  capacity( void ) const
  {
    return data->capacity();
  }

  // Note, in order to use the const version of operator[]
  // through a pointer, it is in some cases necessary to
  // use an explicit TokenArray const * pointer!
  // Use the member function get(size_t) const to force
  // constness.

  Token& operator[]( size_t i )
  {
    clone();
    return ( *data )[ i ];
  }

  const Token& operator[]( size_t i ) const
  {
    return ( *data )[ i ];
  }

  const Token&
  get( long i ) const
  {
    return data->get( i );
  }

  bool
  index_is_valid( long i ) const
  {
    return data->index_is_valid( i );
  }

  void
  rotate( Token* t1, Token* t2, Token* t3 )
  {
    size_t s1 = t1 - data->begin();
    size_t s2 = t2 - data->begin();
    size_t s3 = t3 - data->begin();

    clone();
    Token* b = data->begin();

    data->rotate( b + s1, b + s2, b + s3 );
  }

  void rotate( long n );

  // The following two members shrink and reserve do
  // NOT invoke cloning, since they have no immediate
  // consequences.

  /**
   * Reduce allocated space such that size()==capacity().
   * Returns true if the array was resized and false otherwhise.
   * If true is returned, all existing pointers into the array are
   * invalidated.
   */
  bool
  shrink( void )
  {
    return data->shrink();
  }

  /**
   * Reserve space such that after the call the new capacity is n.
   * Returns true, if the container was reallocated. In this case all
   * existing pointers are invalidated.
   */
  bool
  reserve( size_t n )
  {
    return data->reserve( n );
  }

  unsigned int
  references( void )
  {
    return data->references();
  }

  /**
   * Resizes the container to size s.
   * If the new size is larger than the old size, the new space is initialized
   * with t.
   */
  void
  resize( size_t s, const Token& t = Token() )
  {
    clone();
    data->resize( s, t );
  }

  // Insertion, deletion
  void
  push_back( const Token& t )
  {
    clone();
    data->push_back( t );
  }

  void
  push_back( Datum* d )
  {
    Token t( d );
    clone();
    data->push_back_move( t );
  }

  void
  push_back_move( Token& t )
  {
    clone();
    data->push_back_move( t );
  }

  void
  push_back_dont_clone( Token& t )
  {
    data->push_back_move( t );
  }

  void assign_move( size_t i, Token& t ) // 8.4.98 Diesmann
  {
    clone();
    data->assign_move( data->begin() + i, t );
  }

  void
  assign_move( TokenArray& a, size_t i, size_t n )
  {
    clear(); // no cloning, because we overwrite everything
    // This is slightly inefficient, because if a has references,
    // cloning is more expensive than just copying the desired range.
    if ( a.references() == 1 )
    {
      data->assign_move( *( a.data ), i, n );
    }
    else
    {
      data->assign( *( a.data ), i, n );
    }
  }

  void insert_move( size_t i, TokenArray& a ) // 8.4.98 Diesmann
  {
    clone();   // make copy if others point to representation
    a.clone(); // also for a because we are going to empy it
               //      assert(data->refs==1);    // private copy
               //      assert(a.data->refs==1);  // private copy

    data->insert_move( i, *( a.data ) );
    // the representations insert_move moves the
    // the contens of all Tokens in a.data and marks it empty.

    // assert(a.data->size()==0); // empty, but memory is still allocated incase
    //
    // it will be used again. data->clear() would
    // free the memory. In any case the destructor
    // finally frees the memory.
  }

  void
  insert_move( size_t i, Token& t )
  {
    clone();
    data->insert_move( i, t );
  }


  void
  replace_move( size_t i, size_t n, TokenArray& a )
  {
    clone();
    a.clone();

    data->replace_move( i, n, *( a.data ) );
  }


  void
  append_move( TokenArray& a )
  {
    clone();   // make copy if others point to representation
    a.clone(); // also for a because we are going to empy it

    data->append_move( *( a.data ) );
  }

  void
  pop_back( void )
  {
    clone();
    data->pop_back();
  }

  void
  clear( void )
  {
    erase();
  }

  void
  erase( void )
  {
    if ( not detach() )
    {
      erase( begin(), end() );
    }
  }


  void
  erase( Token* from, Token* to )
  {
    if ( from != to )
    {
      size_t sf = from - data->begin();
      size_t st = to - data->begin();

      clone();
      data->erase( data->begin() + sf, data->begin() + st );
    }
  }

  void
  erase( size_t i, size_t n )
  {
    if ( i < size() && n > 0 )
    {
      clone();
      data->erase( i, n );
    }
  }

  // Reduce the Array to the Range given by the iterators
  void
  reduce( size_t i, size_t n )
  {
    if ( i > 0 || n < size() )
    {
      clone();
      data->reduce( i, n );
    }
  }

  void reverse();

  void
  swap( TokenArray& a )
  {
    std::swap( data, a.data );
  }

  const TokenArray& operator=( const TokenArray& );
  const TokenArray& operator=( const std::vector< long >& );
  const TokenArray& operator=( const std::vector< double >& );

  bool operator==( const TokenArray& a ) const
  {
    return *data == *a.data;
  }

  bool
  empty( void ) const
  {
    return size() == 0;
  }

  void info( std::ostream& ) const;

  /** Fill vectors with homogeneous integer and double arrays */

  void toVector( std::vector< size_t >& ) const;
  void toVector( std::vector< long >& ) const;
  void toVector( std::vector< double >& ) const;
  void toVector( std::vector< std::string >& ) const;

  bool valid( void ) const; // check integrity

  /** Exception classes */
  //  class TypeMismatch {};
  class OutOfRange
  {
  };
};

inline void
TokenArray::reverse()
{
  if ( size() == 0 )
  {
    return;
  }
  clone();
  Token* b = begin();
  Token* e = end() - 1;
  while ( b < e )
  {
    b->swap( *e );
    ++b;
    --e;
  }
}

inline void
TokenArray::rotate( long n = 1 )
{
  if ( size() == 0 || n == 0 )
  {
    return;
  }

  clone();
  long rot = n % static_cast< long >( size() );
  rot = ( rot < 0 ) ? rot + size() : rot;
  std::rotate( begin(), begin() + rot, end() );
}


std::ostream& operator<<( std::ostream&, const TokenArray& );


#ifdef TokenArray_H_DEBUG
#undef TokenArray_H_DEBUG
#undef NDEBUG
#endif


#endif
