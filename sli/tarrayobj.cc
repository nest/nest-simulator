/*
 *  tarrayobj.cc
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

#include "tarrayobj.h"

// Includes from sli:
#include "datum.h"
#include "token.h"

#ifdef assert
#undef assert
#define assert( a )
#endif

size_t TokenArrayObj::allocations = 0;

TokenArrayObj::TokenArrayObj( size_t s, const Token& t, size_t alloc )
  : p( NULL )
  , begin_of_free_storage( NULL )
  , end_of_free_storage( NULL )
  , alloc_block_size( ARRAY_ALLOC_SIZE )
  , refs_( 1 )
{
  size_t a = ( alloc == 0 ) ? s : alloc;

  resize( s, a, t );
}


TokenArrayObj::TokenArrayObj( const TokenArrayObj& a )
  : p( NULL )
  , begin_of_free_storage( NULL )
  , end_of_free_storage( NULL )
  , alloc_block_size( ARRAY_ALLOC_SIZE )
  , refs_( 1 )
{
  if ( a.p != NULL )
  {
    resize( a.size(), a.alloc_block_size, Token() );
    Token* from = a.p;
    Token* to = p;

    while ( to < begin_of_free_storage )
    {
      *to++ = *from++;
    }
  }
}


TokenArrayObj::~TokenArrayObj()
{
  if ( p )
  {
    delete[] p;
  }
}

void
TokenArrayObj::allocate( size_t new_s, size_t new_c, size_t new_a, const Token& t )
{
  // This resize function is private and does an unconditional resize, using
  // all supplied parameters.

  alloc_block_size = new_a;

  size_t old_s = size();

  assert( new_c != 0 );
  assert( new_a != 0 );

  Token* h = new Token[ new_c ];
  assert( h != NULL );

  if ( t != Token() )
  {
    for ( Token* hi = h; hi < h + new_c; ++hi )
    {
      ( *hi ) = t;
    }
  }

  end_of_free_storage = h + new_c; // [,) convention
  begin_of_free_storage = h + new_s;

  if ( p != NULL )
  {

    size_t min_l;

    if ( old_s < new_s )
    {
      min_l = old_s;
    }
    else
    {
      min_l = new_s;
    }

    for ( size_t i = 0; i < min_l; ++i ) // copy old parts
    {
      h[ i ].move( p[ i ] );
    }
    delete[] p;
  }
  p = h;
  assert( p != NULL );

  ++allocations;
}

void
TokenArrayObj::resize( size_t s, size_t alloc, const Token& t )
{
  alloc_block_size = ( alloc == 0 ) ? alloc_block_size : alloc;

  if ( ( s != size() && ( s != 0 ) ) || ( size() == 0 && alloc_block_size != 0 ) )
  {
    allocate( s, s + alloc_block_size, alloc_block_size, t );
  }
}

void
TokenArrayObj::resize( size_t s, const Token& t )
{
  resize( s, alloc_block_size, t );
}

const TokenArrayObj& TokenArrayObj::operator=( const TokenArrayObj& a )
{
  if ( capacity() >= a.size() )
  // This branch also covers the case where a is the null-vector.
  {
    Token* to = begin();
    Token* from = a.begin();
    while ( from < a.end() )
    {
      *to++ = *from++;
    }

    while ( to < end() )
    {
      to->clear();
      to++;
    }
    begin_of_free_storage = p + a.size();

    assert( begin_of_free_storage <= end_of_free_storage );
  }
  else
  {

    if ( p != NULL )
    {
      delete[] p;
      p = NULL;
    }

    resize( a.size(), a.alloc_block_size );
    Token* to = begin();
    Token* from = a.begin();
    while ( from < a.end() )
    {
      *to++ = *from++;
    }
    begin_of_free_storage = to;
    assert( begin_of_free_storage <= end_of_free_storage );
  }

  return *this;
}


// re-allocate, if the actual buffer is larger
// than alloc_block_size

// bool TokenArrayObj::shrink(void)
// {
//     static const size_t hyst=1;
//     size_t old_size = size();

//     size_t n = old_size/alloc_block_size + 1 + hyst;
//     size_t new_capacity = n*alloc_block_size;

//     if( new_capacity < capacity())
//     {
//       allocate(old_size, new_capacity, alloc_block_size);
//       return true;
//     }
//     return false;
// }

bool
TokenArrayObj::shrink( void )
{
  size_t new_capacity = size();

  if ( new_capacity < capacity() )
  {
    allocate( size(), new_capacity, alloc_block_size );
    return true;
  }
  return false;
}

bool
TokenArrayObj::reserve( size_t new_capacity )
{
  if ( new_capacity > capacity() )
  {
    allocate( size(), new_capacity, alloc_block_size );
    return true;
  }
  return false;
}


void
TokenArrayObj::rotate( Token* first, Token* middle, Token* last )
{

  // This algorithm is taken from the HP STL implementation.
  if ( ( first < middle ) && ( middle < last ) )
  {
    for ( Token* i = middle;; )
    {
      first->swap( *i );
      i++;
      first++;

      if ( first == middle )
      {
        if ( i == last )
        {
          return;
        }
        middle = i;
      }
      else if ( i == last )
      {
        i = middle;
      }
    }
  }
}

void
TokenArrayObj::erase( Token* first, Token* last )
{
  // this algorithm we also use in replace_move
  // array is decreasing. we move elements after point of
  // erasure from right to left
  Token* from = last;
  Token* to = first;
  Token* end = begin_of_free_storage; // 1 ahead  as conventional

  while ( from < end )
  {
    if ( to->p )
    {
      // deleting NULL pointer is safe in ISO C++
      to->p->removeReference();
    }
    to->p = from->p; // move
    from->p = NULL;  // might be overwritten or not
    ++from;
    ++to;
  }

  while ( last > to ) // if sequence we have to erase is
  {                   // longer than the sequence to the
    --last;           // right of it, we explicitly delete the
    if ( last->p )
    {
      // elements which are still intact
      last->p->removeReference();
    }
    last->p = NULL; // after the move above.
  }

  begin_of_free_storage = to;
}

// as for strings erase tolerates i+n >=  size()
//
void
TokenArrayObj::erase( size_t i, size_t n )
{
  if ( i + n < size() )
  {
    erase( p + i, p + i + n );
  }
  else
  {
    erase( p + ( i ), p + size() );
  }
}

void
TokenArrayObj::clear( void )
{
  if ( p )
  {
    delete[] p;
  }
  p = begin_of_free_storage = end_of_free_storage = NULL;
  alloc_block_size = 1;
}

// reduce() could be further optimized by testing wether the
// new size leads to a resize. In this case, one could simply
// re-construct the array with the sub-array.

void
TokenArrayObj::reduce( Token* first, Token* last )
{
  assert( last <= end() );
  assert( first >= p );

  // First step: shift all elements to the begin of
  // the array.
  Token* i = p, * j = first;

  if ( first > begin() )
  {
    while ( j < last )
    {
      i->move( *j );
      i++;
      j++;
    }
    assert( j == last );
  }
  else
  {
    i = last;
  }

  assert( i == p + ( last - first ) );

  while ( i < end() )
  {
    i->clear();
    i++;
  }
  begin_of_free_storage = p + ( size_t )( last - first );
  // shrink();
}

// as assign for strings reduce tolerates i+n >= size()
//
void
TokenArrayObj::reduce( size_t i, size_t n )
{
  if ( i + n < size() )
  {
    reduce( p + i, p + i + n );
  }
  else
  {
    reduce( p + ( i ), p + size() );
  }
}

void
TokenArrayObj::insert( size_t i, size_t n, const Token& t )
{
  // pointer argument pos would not be efficient because we
  // have to recompute pointer anyway after reallocation

  reserve( size() + n ); // reallocate if necessary

  Token* pos = p + i;                      // pointer to element i (starting with 0)
  Token* from = begin_of_free_storage - 1; // first Token which has to be moved
  Token* to = from + n;                    // new location of first Token

  while ( from >= pos )
  {
    to->p = from->p; // move
    from->p = NULL;  // knowing that to->p is
    --from;
    --to; // NULL before
  }

  for ( size_t i = 0; i < n; ++i ) // insert n copies of Token t;
  {
    *( pos++ ) = t;
  }

  begin_of_free_storage += n; // new size is old + n
}

void
TokenArrayObj::insert_move( size_t i, TokenArrayObj& a )
{
  reserve( size() + a.size() );                                      // reallocate if necessary
  assert( begin_of_free_storage + a.size() <= end_of_free_storage ); // check

  Token* pos = p + i;                      // pointer to element i (starting with 0)
  Token* from = begin_of_free_storage - 1; // first Token which has to be moved
  Token* to = from + a.size();             // new location of first Token


  while ( from >= pos )
  {
    to->p = from->p; // move
    from->p = NULL;  // knowing that to->p is
    --from;
    --to; // NULL before
  }

  from = a.p;
  to = p + i;

  while ( from < a.end() )
  {
    to->p = from->p; // we cannot do this in the loop
    from->p = NULL;  // above because of overlapping
    ++from;
    ++to;
  }

  begin_of_free_storage += a.size(); // new size is old + n
  a.begin_of_free_storage = a.p;     // a is empty.
}

void
TokenArrayObj::assign_move( TokenArrayObj& a, size_t i, size_t n )
{
  reserve( n );

  Token* from = a.begin() + i;
  Token* end = a.begin() + i + n;
  Token* to = p;

  while ( from < end )
  {
    to->p = from->p;
    from->p = NULL;
    ++from;
    ++to;
  }

  begin_of_free_storage = p + n;
}

void
TokenArrayObj::assign( const TokenArrayObj& a, size_t i, size_t n )
{
  reserve( n );

  Token* from = a.begin() + i;
  Token* end = a.begin() + i + n;
  Token* to = p;

  while ( from < end )
  {
    *to = *from;
    ++from;
    ++to;
  }

  begin_of_free_storage = p + n;
}

void
TokenArrayObj::insert_move( size_t i, Token& t )
{
  reserve( size() + 1 );                                      // reallocate if necessary
  assert( begin_of_free_storage + 1 <= end_of_free_storage ); // check

  Token* pos = p + i;                      // pointer to element i (starting with 0)
  Token* from = begin_of_free_storage - 1; // first Token which has to be moved
  Token* to = from + 1;                    // new location of first Token

  while ( from >= pos )
  {
    to->p = from->p; // move
    from->p = NULL;  // knowing that to->p is
    --from;
    --to; // NULL before
  }

  ( p + i )->p = t.p; // move contens of t
  t.p = NULL;

  begin_of_free_storage += 1; // new size is old + 1
}


void
TokenArrayObj::replace_move( size_t i, size_t n, TokenArrayObj& a )
{
  assert( i < size() ); // assume index in range
  // n more than available is allowed
  n = ( size() - i < n ) ? ( size() - i ) : n;

  long d = a.size() - n; // difference size after the replacement,
                         // positive for increase

  reserve( size() + d ); // reallocate if necessary

  if ( d > 0 )
  {
    // array is increasing. we move elements after point of
    // replacement from left to right
    Token* from = begin_of_free_storage - 1;
    Token* to = begin_of_free_storage - 1 + d;
    Token* end = p + i + n - 1; // 1 ahead (before)  as conventional

    while ( from > end )
    {
      to->p = from->p; // move
      from->p = NULL;  // might be overwritten or not
      --from;
      --to;
    }
  }
  else if ( d < 0 )
  {
    // array is decreasing. we move elements after point of
    // replacement from right to left
    Token* last = p + i + n;
    Token* from = last;
    Token* to = p + i + a.size();
    Token* end = begin_of_free_storage; // 1 ahead  as conventional

    while ( from < end )
    {
      if ( to->p )
      {
        // deleting NULL pointer is safe in ISO C++
        to->p->removeReference();
      }
      to->p = from->p; // move
      from->p = NULL;  // might be overwritten or not
      ++from;
      ++to;
    }

    while ( last > to ) // if sequence we have to erase is
    {                   // longer than a plus the sequence to the
      --last;           // right of it, we explicitly delete the
      if ( last->p )
      {
        // elements which are still intact
        last->p->removeReference();
      }
      last->p = NULL; // after the move above.
    }
  }

  begin_of_free_storage += d; // set new size


  // move contens of array a
  Token* to = p + i;
  Token* end = a.end(); // 1 ahead as conventional
  Token* from = a.begin();

  while ( from < end )
  {
    if ( to->p )
    {
      // delete target before
      to->p->removeReference();
    }
    to->p = from->p; // movement, it is typically
    from->p = NULL;  // not the NULL pointer
    ++from;
    ++to;
  }
}

void
TokenArrayObj::append_move( TokenArrayObj& a )
{
  reserve( size() + a.size() );                                      // reallocate if necessary
  assert( begin_of_free_storage + a.size() <= end_of_free_storage ); // check

  Token* from = a.p;
  Token* to = begin_of_free_storage;

  while ( from < a.end() ) // move
  {                        // knowing that to->p is
    to->p = from->p;       // NULL before
    from->p = NULL;
    ++from;
    ++to;
  }

  begin_of_free_storage += a.size(); // new size is old + n
  a.begin_of_free_storage = a.p;     // a is empty.
}


bool TokenArrayObj::operator==( const TokenArrayObj& a ) const
{

  // std::cout << "comparison of TokenArrayObj" << std::endl;
  // std::cout << "p:   " << p << std::endl;
  // std::cout << "a.p: " << a.p << std::endl;

  if ( p == a.p )
  {
    return true;
  }

  // experimentally replaced by line below 090120, Diesmann
  // because [] cvx has non NULL p
  //
  //    if( p == NULL || a.p == NULL || size() != a.size())
  //    return false;

  if ( size() != a.size() )
  {
    return false;
  }

  Token* i = begin(), * j = a.begin();
  while ( i < end() )
  {
    if ( not( *i++ == *j++ ) )
    {
      return false;
    }
  }
  return true;
}

void
TokenArrayObj::info( std::ostream& out ) const
{
  out << "TokenArrayObj::info\n";
  out << "p    = " << p << std::endl;
  out << "bofs = " << begin_of_free_storage << std::endl;
  out << "eofs = " << end_of_free_storage << std::endl;
  out << "abs  = " << alloc_block_size << std::endl;
}

bool
TokenArrayObj::valid( void ) const
{
  if ( p == NULL )
  {
    std::cerr << "TokenArrayObj::valid: Data pointer missing!" << std::endl;
    return false;
  }

  if ( begin_of_free_storage == NULL )
  {
    std::cerr << "TokenArrayObj::valid: begin of free storage pointer missing!" << std::endl;
    return false;
  }

  if ( end_of_free_storage == NULL )
  {
    std::cerr << "TokenArrayObj::valid: end of free storage pointer missing!" << std::endl;
    return false;
  }

  if ( begin_of_free_storage > end_of_free_storage )
  {
    std::cerr << "TokenArrayObj::valid: begin_of_free_storage  > end_of_free_storage !" << std::endl;
    return false;
  }

  return true;
}


std::ostream& operator<<( std::ostream& out, const TokenArrayObj& a )
{

  for ( Token* i = a.begin(); i < a.end(); ++i )
  {
    out << *i << ' ';
  }

  return out;
}
