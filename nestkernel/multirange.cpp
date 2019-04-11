/*
 *  multirange.cpp
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

#include "multirange.h"

// C++ includes:
#include <stdexcept>

nest::index nest::Multirange::operator[]( index n ) const
{
  for ( RangeVector::const_iterator iter = ranges_.begin(); iter != ranges_.end(); ++iter ) {
    if ( n <= iter->second - iter->first ) {
      return iter->first + n;
    }

    n -= 1 + iter->second - iter->first;
  }
  throw std::out_of_range( "Multirange::operator[]: index out of range." );
}
