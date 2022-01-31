/*
 *  charcode.cc
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

#include "charcode.h"

CharCode::CharCode( size_t n, size_t def )
  : std::vector< size_t >( n + 1, def )
{
}

void
CharCode::Range( size_t code, char lc, char uc )
{
  unsigned char lower = lc;
  unsigned char upper = uc;

  assert( lower <= upper );
  assert( upper < size() );
  for ( size_t i = lower; i <= upper; ++i )
  {
    ( *this )[ i ] = code;
  }
}

void
CharCode::Group( size_t code, const char* g )
{
  while ( *g )
  {
    unsigned char c = *g++;
    assert( c < size() );
    ( *this )[ c ] = code;
  }
}

size_t
CharCode::operator()( char c ) const
{
  unsigned char chr = static_cast< unsigned char >( c );
  assert( chr < size() );

  return ( *this )[ chr ];
}
