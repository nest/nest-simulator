/*
 *  charcode.h
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

#ifndef CharCode_H
#define CharCode_H
/*

    Character codes for the scanner
*/

/***************************************************************/
/* class CharCode                                              */
/***************************************************************/
// C++ includes:
#include <cassert>
#include <cstddef>
#include <vector>

class CharCode : public std::vector< size_t >
{
public:
  CharCode( size_t, size_t );

  void Range( size_t, char, char );
  void Group( size_t, const char* );
  size_t operator()( char ) const;
};


#endif
