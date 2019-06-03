/*
 *  streamers.h
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
#ifndef STREAMERS_H
#define STREAMERS_H

#include <iostream>
#include <vector>

/**
 * General vector streamer
 *
 * This templated operator streams all elements of a std::vector<>.
 */
template < typename T >
std::ostream& operator<<( std::ostream& os, const std::vector< T >& vec )
{
  os << "vector[";
  bool first = true;
  for ( const auto& element : vec )
  {
    if ( not first )
    {
      os << ", ";
    }
    os << element;
    first = false;
  }
  return os << "]";
}

#endif // ndef STREAMERS_H
