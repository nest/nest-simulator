/*
 *  position_impl.h
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

#ifndef POSITION_IMPL_H
#define POSITION_IMPL_H

#include "position.h"

namespace nest
{

template < int D, class T >
inline Position< D, T >::Position()
{
  x_.fill( 0 );
}

template < int D, class T >
inline Position< D, T >::Position( const T& x, const T& y )
{
  assert( D == 2 );
  x_[ 0 ] = x;
  x_[ 1 ] = y;
}

template < int D, class T >
inline Position< D, T >::Position( const T& x, const T& y, const T& z )
{
  assert( D == 3 );
  x_[ 0 ] = x;
  x_[ 1 ] = y;
  x_[ 2 ] = z;
}

template < int D, class T >
inline Position< D, T >::Position( const T* const y )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] = y[ i ];
  }
}

template < int D, class T >
inline Position< D, T >::Position( const std::vector< T >& y )
{
  if ( y.size() != D )
  {
    throw BadProperty( String::compose( "Expected a %1-dimensional position.", D ) );
  }
  std::copy( y.begin(), y.end(), x_.begin() );
}

template < int D, class T >
inline Position< D, T >::Position( const Position< D, T >& other )
  : x_( other.x_ )
{
}

template < int D, class T >
template < class U >
inline Position< D, T >::Position( const Position< D, U >& other )
  : x_( other.x_ )
{
}

template < int D, class T >
inline Position< D, T >::Position( Position&& other )
{
  x_ = std::move( other.x_ );
}

template < int D, class T >
inline Position< D, T >&
Position< D, T >::operator=( const std::vector< T >& y )
{
  if ( y.size() != D )
  {
    throw BadProperty( String::compose( "Expected a %1-dimensional position.", D ) );
  }
  std::copy( y.begin(), y.end(), x_.begin() );

  return *this;
}

template < int D, class T >
inline T&
Position< D, T >::operator[]( int i )
{
  return x_[ i ];
}

template < int D, class T >
inline const T&
Position< D, T >::operator[]( int i ) const
{
  return x_[ i ];
}

template < int D, class T >
Token
Position< D, T >::getToken() const
{
  std::vector< T > result = get_vector();
  return Token( result );
}


template < int D, class T >
const std::vector< T >
Position< D, T >::get_vector() const
{
  return std::vector< T >( x_.begin(), x_.end() );
}

template < int D, class T >
void
Position< D, T >::get_vector( std::vector< T >& vector ) const
{
  assert( vector.size() == D );
  std::copy( x_.begin(), x_.end(), vector.begin() );
}


template < int D, class T >
template < class OT >
inline Position< D, T >
Position< D, T >::operator+( const Position< D, OT >& other ) const
{
  Position p = *this;
  p += other;
  return p;
}

template < int D, class T >
template < class OT >
inline Position< D, T >
Position< D, T >::operator-( const Position< D, OT >& other ) const
{
  Position p = *this;
  p -= other;
  return p;
}

template < int D, class T >
inline Position< D, T >
Position< D, T >::operator-() const
{
  Position p;
  p -= *this;
  return p;
}

template < int D, class T >
template < class OT >
inline Position< D, T >
Position< D, T >::operator*( const Position< D, OT >& other ) const
{
  Position p = *this;
  p *= other;
  return p;
}

template < int D, class T >
template < class OT >
inline Position< D, T >
Position< D, T >::operator/( const Position< D, OT >& other ) const
{
  Position p = *this;
  p /= other;
  return p;
}

template < int D, class T >
inline Position< D, T >
Position< D, T >::operator+( const T& a ) const
{
  Position p = *this;
  p += a;
  return p;
}

template < int D, class T >
inline Position< D, T >
Position< D, T >::operator-( const T& a ) const
{
  Position p = *this;
  p -= a;
  return p;
}

template < int D, class T >
inline Position< D, T >
Position< D, T >::operator*( const T& a ) const
{
  Position p = *this;
  p *= a;
  return p;
}

template < int D, class T >
inline Position< D, T >
Position< D, T >::operator/( const T& a ) const
{
  Position p = *this;
  p /= a;
  return p;
}

template < int D, class T >
template < class OT >
inline Position< D, T >&
Position< D, T >::operator+=( const Position< D, OT >& other )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] += other.x_[ i ];
  }
  return *this;
}

template < int D, class T >
template < class OT >
inline Position< D, T >&
Position< D, T >::operator-=( const Position< D, OT >& other )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] -= other.x_[ i ];
  }
  return *this;
}

template < int D, class T >
template < class OT >
inline Position< D, T >&
Position< D, T >::operator*=( const Position< D, OT >& other )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] *= other.x_[ i ];
  }
  return *this;
}

template < int D, class T >
template < class OT >
inline Position< D, T >&
Position< D, T >::operator/=( const Position< D, OT >& other )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] /= other.x_[ i ];
  }
  return *this;
}

template < int D, class T >
inline Position< D, T >&
Position< D, T >::operator+=( const T& a )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] += a;
  }
  return *this;
}

template < int D, class T >
inline Position< D, T >&
Position< D, T >::operator-=( const T& a )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] -= a;
  }
  return *this;
}

template < int D, class T >
inline Position< D, T >&
Position< D, T >::operator*=( const T& a )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] *= a;
  }
  return *this;
}

template < int D, class T >
inline Position< D, T >&
Position< D, T >::operator/=( const T& a )
{
  for ( int i = 0; i < D; ++i )
  {
    x_[ i ] /= a;
  }
  return *this;
}

template < int D, class T >
inline bool
Position< D, T >::operator==( const Position< D, T >& y ) const
{
  for ( int i = 0; i < D; ++i )
  {
    if ( x_[ i ] != y.x_[ i ] )
    {
      return false;
    }
  }
  return true;
}

template < int D, class T >
inline bool
Position< D, T >::operator!=( const Position< D, T >& y ) const
{
  for ( int i = 0; i < D; ++i )
  {
    if ( x_[ i ] != y.x_[ i ] )
    {
      return true;
    }
  }
  return false;
}

template < int D, class T >
inline bool
Position< D, T >::operator<( const Position< D, T >& y ) const
{
  for ( int i = 0; i < D; ++i )
  {
    if ( x_[ i ] >= y.x_[ i ] )
    {
      return false;
    }
  }
  return true;
}

template < int D, class T >
inline bool
Position< D, T >::operator>( const Position< D, T >& y ) const
{
  for ( int i = 0; i < D; ++i )
  {
    if ( x_[ i ] <= y.x_[ i ] )
    {
      return false;
    }
  }
  return true;
}

template < int D, class T >
inline bool
Position< D, T >::operator<=( const Position< D, T >& y ) const
{
  for ( int i = 0; i < D; ++i )
  {
    if ( x_[ i ] > y.x_[ i ] )
    {
      return false;
    }
  }
  return true;
}

template < int D, class T >
inline bool
Position< D, T >::operator>=( const Position< D, T >& y ) const
{
  for ( int i = 0; i < D; ++i )
  {
    if ( x_[ i ] < y.x_[ i ] )
    {
      return false;
    }
  }
  return true;
}

template < int D, class T >
T
Position< D, T >::length() const
{
  T lensq = 0;
  for ( int i = 0; i < D; ++i )
  {
    lensq += x_[ i ] * x_[ i ];
  }
  return std::sqrt( lensq );
}

template < int D, class T >
Position< D, T >::operator std::string() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

template < int D, class T >
void
Position< D, T >::print( std::ostream& out, char sep ) const
{
  out << x_[ 0 ];
  for ( int i = 1; i < D; ++i )
  {
    out << sep << x_[ i ];
  }
}

template < int D, class T >
std::ostream&
operator<<( std::ostream& os, const Position< D, T >& pos )
{
  os << "(";
  if ( D > 0 )
  {
    os << pos.x_[ 0 ];
  }
  for ( int i = 1; i < D; ++i )
  {
    os << ", " << pos.x_[ i ];
  }
  os << ")";
  return os;
}

}

#endif
